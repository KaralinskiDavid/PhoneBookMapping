#include "pch.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <locale>

#include "PhoneBookLine.h"


using namespace std;

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

Node<wstring, PhoneBookLine>* StreetIndex = NULL;
Node<wstring, PhoneBookLine>* LastnameIndex = NULL;
Node<wstring, PhoneBookLine>* PhonenumberIndex = NULL;


extern _declspec(dllexport) vector<PhoneBookLine*> loadPhonebook(wstring path)
{
	wifstream in(path);
	vector<wstring> substrings;
	wstring line;
	wstring delimiter = wstring(L" ");
	PhoneBookLine* phonebookElement;
	vector<PhoneBookLine*> phonebook;

	setlocale(LC_ALL, "C");

	if (in.is_open())
	{
		while (getline(in, line))
		{
			try 
			{
				substrings = splitline(line, delimiter);
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
				phonebook.push_back(phonebookElement);
				StreetIndex->insert(&StreetIndex, phonebookElement->street, phonebookElement);
				LastnameIndex->insert(&LastnameIndex, phonebookElement->lastname, phonebookElement);
				PhonenumberIndex->insert(&PhonenumberIndex, phonebookElement->phonenumber, phonebookElement);
			}
			catch(...)
			{

			}
		}

		in.close();

		return phonebook;
	}

}

typedef enum IndexTypes
{
	LASTNAME_INDEX,
	STREET_INDEX,
	PHONENUMBER_INDEX
} IndexTypes;


extern _declspec(dllexport) vector<PhoneBookLine*> searchByIndex(wstring index, int indextype)
{
	switch (indextype)
	{
	case STREET_INDEX:
		return StreetIndex->getNodesByKey(StreetIndex, index);
	case LASTNAME_INDEX:
		return LastnameIndex->getNodesByKey(LastnameIndex, index);
	case PHONENUMBER_INDEX:
		return PhonenumberIndex->getNodesByKey(PhonenumberIndex, index);
	}
}

void destroy(Node<wstring, PhoneBookLine>* node, bool isFirst=false)
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
