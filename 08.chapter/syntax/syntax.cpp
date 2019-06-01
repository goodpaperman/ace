// syntax.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

class Base
{
public:
  Base() 
  {
    printf("Base constructed.\n"); 
    open(); 
  }

  virtual ~Base()
  {
    close(); 
    printf("Base destructed.\n"); 
  }

  virtual int open()
  {
    printf("Base open.\n"); 
    return 0; 
  }

  virtual int close()
  {
    printf("Base close.\n"); 
    return 0; 
  }
}; 

class Derived : public Base
{
public:
  Derived()
  {
    printf("Derived constructed.\n"); 
    open(); 
  }

  virtual ~Derived()
  {
    close(); 
    printf("Derived destructed.\n"); 
  }

  virtual int open()
  {
    printf("Derived open.\n"); 
    return 0; 
  }

  virtual int close()
  {
    printf("Derived close.\n"); 
    return 0; 
  }
}; 


int _tmain(int argc, _TCHAR* argv[])
{
  //Base base; 
  //Derived derived; 
  Base* base = new Derived; 
  delete base; 
	return 0;
}

