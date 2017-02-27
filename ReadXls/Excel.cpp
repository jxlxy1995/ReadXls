
//Excel.cpp
#include "stdafx.h"
#include <tchar.h>
#include "Excel.h"


COleVariant
covTrue((short)TRUE),
covFalse((short)FALSE),
covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

CApplication Excel::application;

Excel::Excel() :isLoad(false)
{
}


Excel::~Excel()
{
	//close();
}


bool Excel::initExcel()
{
	//����Excel 2000������(����Excel)   
	if (!application.CreateDispatch(_T("Excel.application"), nullptr))
	{
		MessageBox(nullptr, _T("����Excel����ʧ��,�����û�а�װEXCEL������!"), _T("����"), MB_OK);
		return FALSE;
	}

	application.put_DisplayAlerts(FALSE);
	return true;
}


void Excel::release()
{
	application.Quit();
	application.ReleaseDispatch();
	application = nullptr;
}

bool Excel::open(const char* fileName)
{

	//�ȹر��ļ�
	close();

	//����ģ�彨�����ĵ�
	books.AttachDispatch(application.get_Workbooks(), true);


	LPDISPATCH lpDis = nullptr;
	lpDis = books.Add(COleVariant(CString(fileName)));

	if (lpDis)
	{
		workBook.AttachDispatch(lpDis);

		sheets.AttachDispatch(workBook.get_Worksheets());

		openFileName = fileName;
		return true;
	}

	return false;
}

void Excel::close(bool ifSave)
{
	//����ļ��Ѿ��򿪣��ر��ļ�
	if (!openFileName.IsEmpty())
	{
		//�������,�����û�����,���û��Լ��棬����Լ�SAVE�������Ī���ĵȴ�  
		if (ifSave)
		{
			//show(true);
		}
		else
		{
			workBook.Close(COleVariant(short(FALSE)), COleVariant(openFileName), covOptional);
			books.Close();
		}

		//��մ��ļ�����
		openFileName.Empty();
	}


	sheets.ReleaseDispatch();
	workSheet.ReleaseDispatch();
	currentRange.ReleaseDispatch();
	workBook.ReleaseDispatch();
	books.ReleaseDispatch();
}

void Excel::saveAsXLSFile(const CString &xlsFile)
{
	workBook.SaveAs(COleVariant(xlsFile),
		covOptional,
		covOptional,
		covOptional,
		covOptional,
		covOptional,
		0,
		covOptional,
		covOptional,
		covOptional,
		covOptional,
		covOptional);
	return;
}


int Excel::getSheetCount()
{
	return sheets.get_Count();
}

CString Excel::getSheetName(long tableID)
{
	CWorksheet sheet;
	sheet.AttachDispatch(sheets.get_Item(COleVariant((long)tableID)));
	CString name = sheet.get_Name();
	sheet.ReleaseDispatch();
	return name;
}


void Excel::preLoadSheet()
{
	CRange used_range;

	used_range = workSheet.get_UsedRange();


	VARIANT ret_ary = used_range.get_Value2();
	if (!(ret_ary.vt & VT_ARRAY))
	{
		return;
	}
	//  
	safeArray.Clear();
	safeArray.Attach(ret_ary);
}

//�������Ƽ���sheet����Ҳ����ǰ�������б���
bool Excel::loadSheet(long tableId, bool preLoaded)
{
	LPDISPATCH lpDis = nullptr;
	currentRange.ReleaseDispatch();
	currentRange.ReleaseDispatch();
	lpDis = sheets.get_Item(COleVariant((long)tableId));
	if (lpDis)
	{
		workSheet.AttachDispatch(lpDis, true);
		currentRange.AttachDispatch(workSheet.get_Cells(), true);
	}
	else
	{
		return false;
	}

	isLoad = false;
	//�������Ԥ�ȼ���  
	if (preLoaded)
	{
		preLoadSheet();
		isLoad = true;
	}

	return true;
}


bool Excel::loadSheet(CString sheet, bool preLoaded)
{
	LPDISPATCH lpDis = nullptr;
	currentRange.ReleaseDispatch();
	currentRange.ReleaseDispatch();

	lpDis = sheets.get_Item(COleVariant(sheet));
	if (lpDis)
	{
		workSheet.AttachDispatch(lpDis, true);
		currentRange.AttachDispatch(workSheet.get_Cells(), true);
	}
	else
	{
		return false;
	}

	isLoad = false;
	//�������Ԥ�ȼ���  
	if (preLoaded)
	{
		preLoadSheet();
		isLoad = true;
	}

	return true;
}


int Excel::getColumnCount()
{
	CRange range;
	CRange usedRange;

	usedRange.AttachDispatch(workSheet.get_UsedRange(), true);
	range.AttachDispatch(usedRange.get_Columns(), true);
	int count = range.get_Count();

	usedRange.ReleaseDispatch();
	range.ReleaseDispatch();

	return count;
}

int Excel::getRowCount()
{
	CRange range;
	CRange usedRange;

	usedRange.AttachDispatch(workSheet.get_UsedRange(), true);
	range.AttachDispatch(usedRange.get_Rows(), true);

	int count = range.get_Count();

	usedRange.ReleaseDispatch();
	range.ReleaseDispatch();

	return count;
}

bool Excel::isCellString(long iRow, long iColumn)
{
	CRange range;
	range.AttachDispatch(currentRange.get_Item(COleVariant((long)iRow), COleVariant((long)iColumn)).pdispVal, true);
	COleVariant vResult = range.get_Value2();
	//VT_BSTR��ʾ�ַ���  
	if (vResult.vt == VT_BSTR)
	{
		return true;
	}
	return false;
}


bool Excel::isCellInt(long iRow, long iColumn)
{

	CRange range;
	range.AttachDispatch(currentRange.get_Item(COleVariant((long)iRow), COleVariant((long)iColumn)).pdispVal, true);
	COleVariant vResult = range.get_Value2();
	//VT_BSTR��ʾ�ַ���  
	if (vResult.vt == VT_INT || vResult.vt == VT_R8)
	{
		return true;
	}
	return false;
}

CString Excel::getCellString(long iRow, long iColumn)
{
	COleVariant vResult;
	CString str;
	//�ַ���  
	if (isLoad == false)
	{
		CRange range;
		range.AttachDispatch(currentRange.get_Item(COleVariant((long)iRow), COleVariant((long)iColumn)).pdispVal, true);
		vResult = range.get_Value2();
		range.ReleaseDispatch();
	}
	//�����������Ԥ�ȼ�����  
	else
	{
		long read_address[2];
		VARIANT val;
		read_address[0] = iRow;
		read_address[1] = iColumn;
		safeArray.GetElement(read_address, &val);
		vResult = val;
	}

	if (vResult.vt == VT_BSTR)
	{
		str = vResult.bstrVal;
	}
	//����  
	else if (vResult.vt == VT_INT)
	{
		str.Format(_T("%d"), vResult.pintVal);
	}
	//8�ֽڵ�����   
	else if (vResult.vt == VT_R8)
	{
		if ( ((int)(vResult.dblVal * 100) % 100) > 0)
		{
			str.Format(_T("%.2f"), vResult.dblVal);
		}
		else
		{
			str.Format(_T("%.0f"), vResult.dblVal);
		}
	}
	//ʱ���ʽ  
	else if (vResult.vt == VT_DATE)
	{
		SYSTEMTIME st;
		VariantTimeToSystemTime(vResult.date, &st);
		CTime tm(st);
		str = tm.Format(_T("%Y-%m-%d"));

	}
	//��Ԫ��յ�  
	else if (vResult.vt == VT_EMPTY)
	{
		str = "";
	}
	//δ֪����
// 	{
// 		str = vResult.bstrVal;
// 	}

	return str;
}

double Excel::getCellDouble(long iRow, long iColumn)
{
	double rtn_value = 0;
	COleVariant vresult;
	//�ַ���  
	if (isLoad == false)
	{
		CRange range;
		range.AttachDispatch(currentRange.get_Item(COleVariant((long)iRow), COleVariant((long)iColumn)).pdispVal, true);
		vresult = range.get_Value2();
		range.ReleaseDispatch();
	}
	//�����������Ԥ�ȼ�����  
	else
	{
		long read_address[2];
		VARIANT val;
		read_address[0] = iRow;
		read_address[1] = iColumn;
		safeArray.GetElement(read_address, &val);
		vresult = val;
	}

	if (vresult.vt == VT_R8)
	{
		rtn_value = vresult.dblVal;
	}

	return rtn_value;
}

int Excel::getCellInt(long iRow, long iColumn)
{
	int num;
	COleVariant vresult;

	if (isLoad == FALSE)
	{
		CRange range;
		range.AttachDispatch(currentRange.get_Item(COleVariant((long)iRow), COleVariant((long)iColumn)).pdispVal, true);
		vresult = range.get_Value2();
		range.ReleaseDispatch();
	}
	else
	{
		long read_address[2];
		VARIANT val;
		read_address[0] = iRow;
		read_address[1] = iColumn;
		safeArray.GetElement(read_address, &val);
		vresult = val;
	}
	//  
	num = static_cast<int>(vresult.dblVal);

	return num;
}

void Excel::setCellString(long iRow, long iColumn, CString newString)
{
	COleVariant new_value(newString);
	CRange start_range = workSheet.get_Range(COleVariant(_T("A1")), covOptional);
	CRange write_range = start_range.get_Offset(COleVariant((long)iRow - 1), COleVariant((long)iColumn - 1));
	write_range.put_Value2(new_value);
	start_range.ReleaseDispatch();
	write_range.ReleaseDispatch();
}

void Excel::setCellInt(long iRow, long iColumn, int newInt)
{
	COleVariant new_value((long)newInt);
	CRange start_range = workSheet.get_Range(COleVariant(_T("A1")), covOptional);
	CRange write_range = start_range.get_Offset(COleVariant((long)iRow - 1), COleVariant((long)iColumn - 1));
	write_range.put_Value2(new_value);
	start_range.ReleaseDispatch();
	write_range.ReleaseDispatch();
}


void Excel::show(bool bShow)
{
	application.put_Visible(bShow);
	application.put_UserControl(bShow);
}

CString Excel::getOpenFileName()
{
	return openFileName;
}

CString Excel::getOpenSheelName()
{
	return workSheet.get_Name();
}

char* Excel::getColumnName(long iColumn)
{
	static char column_name[64];
	size_t str_len = 0;

	while (iColumn > 0)
	{
		int num_data = iColumn % 26;
		iColumn /= 26;
		if (num_data == 0)
		{
			num_data = 26;
			iColumn--;
		}
		column_name[str_len] = (char)((num_data - 1) + 'A');
		str_len++;
	}
	column_name[str_len] = '\0';
	//��ת  
	_strrev(column_name);

	return column_name;
}
