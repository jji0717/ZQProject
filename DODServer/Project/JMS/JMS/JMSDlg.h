// JMSDlg.h : ͷ�ļ�
//

#pragma once
//#include"jmsobject.h"
#include"ActiveJMS.h"

// CJMSDlg �Ի���
class CJMSDlg : public CDialog
{
// ����
public:
	CJMSDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_JMS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
    CActiveJMS   jms;

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
