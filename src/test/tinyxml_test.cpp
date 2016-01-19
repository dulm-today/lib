#include <iostream>
#include "tinyxml.h"
using namespace std;


void CreateXml()
{
	char floader[200],buffer[200];
    
    // 1
    TiXmlDocument* m_xmlDoc = new TiXmlDocument();
    
    // 2
    TiXmlElement xElement("player");
    sprintf(buffer,"%d", 1);
    xElement.SetAttribute("admin", buffer);
    
    // 3
    TiXmlElement xPos("pos");
    sprintf(buffer,"%d",2);
    xPos.SetAttribute("x", buffer);
    sprintf(buffer,"%d",3);
    xPos.SetAttribute("y", buffer);
    sprintf(buffer,"%d",4);
    xPos.SetAttribute("zone", buffer);
    
    // 4¡¢5
    xElement.InsertEndChild(xPos);
    m_xmlDoc->InsertEndChild(xElement);
    
    // 6
    sprintf(floader,"%s.xml", "antking");
    m_xmlDoc->SaveFile(floader);
    
    // 7
    delete m_xmlDoc;
}

void ShowXml()
{
	char floader[200];
    int admin1,x1,y1,z1;
    
    // 1
    TiXmlDocument* m_xmlDoc;
    sprintf(floader,"%s.xml", "antking");
    m_xmlDoc = new TiXmlDocument(floader);
    
    if (m_xmlDoc->LoadFile())
    {
        
        // 2
        TiXmlElement *xPlayer = 0;
        xPlayer = m_xmlDoc->FirstChildElement("player");
        
        if (xPlayer)
        {
            
            if (xPlayer->Attribute("admin"))
                admin1= (bool)atoi(xPlayer->Attribute("admin"));
            
            // 3 
            TiXmlElement *xZone = 0;
            xZone = xPlayer->FirstChildElement("pos");
            
            x1 = (int)atoi(xZone->Attribute("x"));
            y1 = (int)atoi(xZone->Attribute("y"));
            z1 = (int)atoi(xZone->Attribute("zone"));
            
        }
        
    }
    
    // 4 
    delete m_xmlDoc;
    
    printf("isAdmin = %d\nx1 = %d, y1 = %d, z1 = %d\n",admin1,x1,y1,z1); 
}

void usage(int code)
{
	printf("%s create|show\n");
	
	exit(code);
}

int main(int argc, const char** argv)
{
	if (argc < 2)
		usage(1);
	
	if (0 == strcmp(argv[1], "create"))
		CreateXml();
	else if (0 == strcmp(argv[1], "show"))
		ShowXml();
	else
		usage(1);
		
	return 0;
}