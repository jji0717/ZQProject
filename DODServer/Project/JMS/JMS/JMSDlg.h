// JMSDlg.h : 头文件
//

#pragma once
//#include"jmsobject.h"
#include"ActiveJMS.h"

// CJMSDlg 对话框
class CJMSDlg : public CDialog
{
// 构造
public:
	CJMSDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_JMS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    CActiveJMS   jms;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CString m_QueueName;
	CString m_TopicName;
	CString m_SendString;
	afx_msg void OnBnClickedButton3();
};
