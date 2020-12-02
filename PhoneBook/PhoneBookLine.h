#pragma once

#include<string>

using namespace std;

class PhoneBookLine {
public:
	PhoneBookLine(wstring _phonenumber, wstring _lastname, wstring _firstname, wstring _surname, wstring _street,
		int _house, int _corpus, int _flat)
	{
		phonenumber = _phonenumber;
		lastname = _lastname;
		firstname = _firstname;
		surname = _surname;
		street = _street;
		house = _house;
		corpus = _corpus;
		flat = _flat;
	}

	wstring phonenumber;
	wstring lastname;
	wstring firstname;
	wstring surname;
	wstring street;
	int house;
	int corpus;
	int flat;

};
