﻿#include "pch.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <locale>
#include <algorithm>
#include "PhoneBookLine.h"

#define FRAME_SIZE 65536
#define LINES_ON_PAGE 30


using namespace std;

bool initialized = false;
wstring _path;

class FilePosition
{
public:
	unsigned int pageNumber;
	unsigned int outerOffset;
	unsigned int innerOffset;
	unsigned int lineSize;
	FilePosition(int pageNumber, int outerOffset, int innerOffset, int lineSize);
};

FilePosition::FilePosition(int pageNumber, int outerOffset, int innerOffset, int lineSize)
{
	this->pageNumber = pageNumber;
	this->outerOffset = outerOffset;
	this->innerOffset = innerOffset;
	this->lineSize = lineSize;
}

template<typename K, typename D>
class Node {
public:
	K key;
	D* data;
	vector<D*> samekeydata;

	Node* left;
	Node* right;
	Node* parent;
	Node* newNode(K key, D* data, Node* parent);
	void insert(Node** head, K key, D* data);
	vector<D*> getNodesByKey(Node* root, K key);
};


template<typename K, typename D>
Node<K,D>* Node<K, D>::newNode(K key, D* data, Node* parent)
{
	Node* tmp = new Node<K,D>();
	tmp->left = tmp->right = NULL;
	tmp->parent = parent;
	tmp->key = key;
	tmp->data = (D*)malloc(sizeof(D));
	tmp->data = data;
	return tmp;
}

template<typename K, typename D>
void Node<K, D>::insert(Node** head, K key, D* data)
{
	Node* tmp = NULL;
	Node* ins = NULL;
	if (*head == NULL)
	{
		*head = Node::newNode(key, data, NULL);
		return;
	}

	tmp = *head;
	while (tmp)
	{
		if (key > tmp->key)
		{
			if (tmp->right)
			{
				tmp = tmp->right;
				continue;
			}
			else
			{
				tmp->right = Node::newNode(key, data, tmp);
				return;
			}
		}
		else if (key < tmp->key)
		{
			if (tmp->left)
			{
				tmp = tmp->left;
				continue;
			}
			else
			{
				tmp->left = Node::newNode(key, data, tmp);
				return;
			}
		}
		else if (key == tmp->key)
		{
			tmp->samekeydata.push_back(data);
			return;
		}
	}

}

template<typename K, typename D>
vector<D*> Node<K,D>::getNodesByKey(Node* root, K key)
{
	vector<D*> lines;
	while (root)
	{
		if (root->key > key)
		{
			root = root->left;
			continue;
		}
		else if (root->key < key)
		{
			root = root->right;
			continue;
		}
		else
		{
			lines = root->samekeydata;
			lines.push_back(root->data);
			return lines;
		}
	}
	return lines;
}

vector<wstring> splitline(wstring line, wstring delimiter)
{
	vector<wstring> substrings;
	size_t prev = 0;
	size_t next;
	size_t delta = delimiter.length();

	while ((next = line.find(delimiter, prev)) != string::npos)
	{
		substrings.push_back(line.substr(prev, next - prev));
		prev = next + delta;
	}
	substrings.push_back(line.substr(prev));

	return substrings;
}

PhoneBookLine* getPhoneBookLine(wstring string)
{
	wstring delimiter = wstring(L" ");
	vector<wstring> substrings;
	PhoneBookLine* phonebookElement = NULL;
	try
	{
		substrings = splitline(string, delimiter);
		phonebookElement = new PhoneBookLine(
			substrings[0],
			substrings[1],
			substrings[2],
			substrings[3],
			substrings[4],
			_wtoi(substrings[5].c_str()),
			_wtoi(substrings[6].c_str()),
			_wtoi(substrings[7].c_str())
		);
	}
	catch (...)
	{

	}
	return phonebookElement;
}

PhoneBookLine* getLineByPosition(FilePosition* position)
{
	PhoneBookLine* phoneBookLine;
	HANDLE hFile = CreateFile(_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	int fileSize = GetFileSize(hFile, NULL);
	HANDLE hMem = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, fileSize, NULL);

	try {
		int outerOffset = position->outerOffset;
		int innerOffset = position->innerOffset;
		int lineSize = position->lineSize;
		int frameMappingSize = outerOffset + FRAME_SIZE > fileSize ? fileSize - outerOffset : FRAME_SIZE;
		char* mappedTextPtr = (char*)MapViewOfFile(hMem, FILE_MAP_READ, 0, outerOffset, frameMappingSize);
		wstring ws(&mappedTextPtr[innerOffset], &mappedTextPtr[innerOffset + lineSize]);
		phoneBookLine = getPhoneBookLine(ws);
		UnmapViewOfFile(mappedTextPtr);
	}
	catch(...)
	{ }

	CloseHandle(hFile);
	CloseHandle(hMem);
	return phoneBookLine;
}

Node<size_t, FilePosition>* StreetIndex = NULL;
Node<size_t, FilePosition>* LastnameIndex = NULL;
Node<size_t, FilePosition>* PhonenumberIndex = NULL;


extern _declspec(dllexport) vector<PhoneBookLine*> loadPhonebook(wstring path, int page)
{
	_path = path;
	PhoneBookLine* phonebookElement;
	vector<PhoneBookLine*> phonebook;

	HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	int fileSize = GetFileSize(hFile, NULL);
	HANDLE hMem = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, fileSize, NULL);

	int fileMappingOffset = 0;
	int linesCount = 0;
	vector<wstring> records;
	hash<wstring> hash;
	try {
		while (fileMappingOffset < fileSize)
		{
			int frameMappingSize = fileMappingOffset + FRAME_SIZE > fileSize ? fileSize - fileMappingOffset : FRAME_SIZE;
			char* mappedTextPtr = (char*)MapViewOfFile(hMem, FILE_MAP_READ, 0, fileMappingOffset, frameMappingSize);
			int offset = 0;
			while (offset < frameMappingSize)
			{
				int i = 0;
				while (offset + i < frameMappingSize && mappedTextPtr[offset + i] != '\n')
				{
					i++;
				}
				wstring ws(&mappedTextPtr[offset], &mappedTextPtr[offset + i]);
				phonebookElement = getPhoneBookLine(ws);
				int currentPage = linesCount / LINES_ON_PAGE;
				if (currentPage == page)
					phonebook.push_back(phonebookElement);
				if (!initialized)
				{
					FilePosition* filePostition = new FilePosition(currentPage, fileMappingOffset, offset, i);
					StreetIndex->insert(&StreetIndex, hash(phonebookElement->street), filePostition);
					LastnameIndex->insert(&LastnameIndex, hash(phonebookElement->lastname), filePostition);
					PhonenumberIndex->insert(&PhonenumberIndex, hash(phonebookElement->phonenumber), filePostition);
				}
				linesCount++;
				offset += i + 1;
			}
			fileMappingOffset += frameMappingSize;
			UnmapViewOfFile(mappedTextPtr);
		}
	}
	catch(...)
	{ }
	CloseHandle(hMem);
	CloseHandle(hFile);

	initialized = true;
	return phonebook;
}

typedef enum IndexTypes
{
	LASTNAME_INDEX,
	STREET_INDEX,
	PHONENUMBER_INDEX
} IndexTypes;

vector<PhoneBookLine*> searchByFilePostition(vector<FilePosition*> positions, int page)
{
	vector<PhoneBookLine*> lines;
	int lowLimit = page * LINES_ON_PAGE;
	int highLimit = positions.size() > lowLimit + LINES_ON_PAGE ? lowLimit + LINES_ON_PAGE : positions.size();
	for (int i = lowLimit; i < highLimit; i++)
	{
		lines.push_back(getLineByPosition(positions[i]));
	}
	return lines;
}


extern _declspec(dllexport) vector<PhoneBookLine*> searchByIndex(wstring index, int indextype, int page)
{
	hash<wstring> hash;
	switch (indextype)
	{
		case STREET_INDEX:
		{
			return searchByFilePostition(StreetIndex->getNodesByKey(StreetIndex, hash(index)), page);
		}
		case LASTNAME_INDEX:
		{
			return searchByFilePostition(LastnameIndex->getNodesByKey(LastnameIndex, hash(index)),page);
		}
		case PHONENUMBER_INDEX:
		{
			return searchByFilePostition(PhonenumberIndex->getNodesByKey(PhonenumberIndex, hash(index)),page);
		}
	}
}

void destroy(Node<size_t, FilePosition>* node, bool isFirst=false)
{
	if (node != NULL)
	{
		if (node->left != NULL)
		{
			destroy(node->left);
		}
		if (node->right != NULL)
		{
			destroy(node->right);
		}
		if (isFirst && node->data!=NULL)
		{
			free(node->data);
			node->data = NULL;
		}
		free(node);
		node = NULL;
	}
	return;
}

extern _declspec(dllexport) void destroyPhoneBook()
{
	destroy(StreetIndex, true);
	destroy(LastnameIndex);
	destroy(PhonenumberIndex);
	return;
}
