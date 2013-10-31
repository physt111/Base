#include <string>

using namespace std;

#ifndef SIMPLE_ERROR_CLASS
#define SIMPLE_ERROR_CLASS

/// просто описывает ошибку
class simple_error
{
public:

	int ID;
	string description;

	simple_error()
	{
		ID = 0;
		description = "undefined error";
	};

	simple_error(string desc, int id)
	{
		ID = id;
		description = desc;
	};
};

#endif
