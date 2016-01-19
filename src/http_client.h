#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#ifndef __HTTP_CURL_H__  
#define __HTTP_CURL_H__

#define HTTP_CONNECCT_TIMEOUT	(3)
#define HTTP_RESPONSE_TIMEOUT	(3)
#define HTTP_MAX_TIMEOUT		(15*60)
  
#include <string>  
  
class CHttpClient  
{  
public:  
    CHttpClient(void);  
    ~CHttpClient(void);  
  
public:  
    /** 
    * @brief HTTP POST���� 
    * @param strUrl �������,�����Url��ַ,��:http://www.baidu.com 
    * @param strPost �������,ʹ�����¸�ʽpara1=val1?2=val2&�� 
    * @param strResponse �������,���ص����� 
    * @return �����Ƿ�Post�ɹ� 
    */  
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);  
  
    /** 
    * @brief HTTP GET���� 
    * @param strUrl �������,�����Url��ַ,��:http://www.baidu.com 
    * @param strResponse �������,���ص����� 
    * @return �����Ƿ�Post�ɹ� 
    */  
    int Get(const std::string & strUrl, std::string & strResponse);  
  
    /** 
    * @brief HTTPS POST����,��֤��汾 
    * @param strUrl �������,�����Url��ַ,��:https://www.alipay.com 
    * @param strPost �������,ʹ�����¸�ʽpara1=val1?2=val2&�� 
    * @param strResponse �������,���ص����� 
    * @param pCaPath �������,ΪCA֤���·��.�������ΪNULL,����֤��������֤�����Ч��. 
    * @return �����Ƿ�Post�ɹ� 
    */  
    int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);  
  
    /** 
    * @brief HTTPS GET����,��֤��汾 
    * @param strUrl �������,�����Url��ַ,��:https://www.alipay.com 
    * @param strResponse �������,���ص����� 
    * @param pCaPath �������,ΪCA֤���·��.�������ΪNULL,����֤��������֤�����Ч��. 
    * @return �����Ƿ�Post�ɹ� 
    */  
    int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);  
  
public:  
    void SetDebug(bool bDebug);
	void SetConnTimeout(int timeout);
	void SetTimeout(int timeout);
  
private:
	int		m_nconn_timeout;
	int		m_ntimeout;
    bool 	m_bDebug;  
}; 

#endif //__HTTP_CURL_H__

#endif //HTTP_CLIENT_H

