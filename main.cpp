#include "unstd.h"
#include <iostream>


using namespace unstd;


int main()
{
	sptr<int> s = shared<int>(1);
	
	try
	{
		throw Exception("what what ", 123, " that's some bullshit!");
	}
	catch (Exception &e)
	{
		std::cout << "Get error: " << e.what() << std::endl;
	}
	
	return 1;
}


