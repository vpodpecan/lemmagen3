/******************************************************************************
This file is part of the lemmagen library. It gives support for lemmatization.
Copyright (C) 2006-2007 Matjaz Jursic <matjaz@gmail.com>
Minor modifications for a working Python extension by Vid Podpecan <vid.podpecan@ijs.si>

The lemmagen library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#include "definitions.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;



PYBIND11_MODULE(_lemmagen, m) {
    py::class_<RdrLemmatizer>(m, "RdrLemmatizer")
        .def(py::init<const char *>(), "Creates a RdrLemmatizer and loads the given model")
 		.def(py::init(), "Creates an empty RdrLemmatizer")
        .def("load_model", &RdrLemmatizer::LoadBinary, "Loads a binary model")
        .def("lemmatize", &RdrLemmatizer::Lemmatize, "Lemmatizes given word");
}


//-------------------------------------------------------------------------------------------
//constructors
RdrLemmatizer::RdrLemmatizer(const char *acFileName) {
	LoadBinary(acFileName);
}

RdrLemmatizer::RdrLemmatizer() {
	this->abData = (unsigned char*) abDataStatic;
	this->iDataLen = iDataLenStatic;
}

//-------------------------------------------------------------------------------------------
//destructor
RdrLemmatizer::~RdrLemmatizer() {
	if (this->abData != (unsigned char*) abDataStatic)
		delete[] abData;
}

//-------------------------------------------------------------------------------------------
//returns size of a tree (number of bytes)
int RdrLemmatizer::SizeOfTree() const{
	return iDataLen - DataStart;
}

//-------------------------------------------------------------------------------------------
//lematizes word according to this data
char *RdrLemmatizer::Lemmatize(const char *acWord) const{
	unsigned char bWordLen = strlen(acWord);

	dword iAddr = DataStart;
	dword iParentAddr = DataStart;
	dword iTmpAddr;
	char bLookChar = bWordLen;
	unsigned char bType = abData[iAddr];

    if (this->abData == (unsigned char*) abDataStatic) {
        throw std::runtime_error("Cannot lemmatize: please load a model first.");
    }

	while(true) {
		iTmpAddr = iAddr+FlagLen+AddrLen;

		//check if additional characters match
		if ((bType & BitAddChar) == BitAddChar) {
			unsigned char bNewSufxLen = abData[iTmpAddr];
			iTmpAddr += LenSpecLen;

			bLookChar -= bNewSufxLen;

			//test additional chars if ok
			if (bLookChar>=0)
				do bNewSufxLen--;
				while (bNewSufxLen!=255 && abData[iTmpAddr+bNewSufxLen] == (unsigned char) acWord[bLookChar+bNewSufxLen]);

			//wrong node, take parents rule
			if (bNewSufxLen!=255) {	iAddr = iParentAddr; break; }

			//right node, but we are at the end (there will be no new loop) finish by returning this rule
			if ((bType & ~BitEntireWr) == TypeLeafAC) break;

			//right node and we need to go on with subnodes (it si probably type TypeIntrAC )
			//set iTmpAddr to start of hashtable
			iTmpAddr += abData[iTmpAddr-LenSpecLen];
		}

		//move lookup char back
		bLookChar--;
		//check if we are still inside the word (bLookChar==0 when at the begining of word)
		if (bLookChar<0) {
			//this means that we are just one character in front of the word so we must look for entireword entries
			if((bType & BitInternal) == BitInternal) {
				//go to the hashtable position 0(NULL) and look idf address is not NULL
				iTmpAddr += ModLen;
				unsigned char bChar = abData[iTmpAddr];
				GETDWORD(,iTmpAddr,iTmpAddr+CharLen);
				if (bChar == 0 && iTmpAddr!=0) {
					//we have a candidate for entireword, redirect addresses
					iParentAddr = iAddr;
					iAddr = iTmpAddr;
					bType = abData[iAddr];
					//increase lookchar (because we actualy eat one character)
					bLookChar++;
				}
			}
			break;
		}

		//find best node in hash table
		if((bType & BitInternal) == BitInternal) {
			unsigned char bMod = abData[iTmpAddr];
			unsigned char bChar = acWord[bLookChar];

			iTmpAddr += ModLen + (bChar%bMod)*(AddrLen+CharLen);

			iTmpAddr = abData[iTmpAddr] == bChar ? iTmpAddr + CharLen : iAddr + FlagLen;

			iParentAddr = iAddr;
			GETDWORD(,iAddr, iTmpAddr);
			bType = abData[iAddr];

			if ((bType & ~BitEntireWr) == TypeRule) break;
		}
	}
	//if this is entire-word node, and we are not at the begining of word it's wrong node - take parents
	if((bType & BitEntireWr) == BitEntireWr && bLookChar!=0) {
		iAddr = iParentAddr;
		bType = abData[iAddr];
	}

	//search ended before we came to te node of type rule but current node is OK so find it's rule node
	if((bType & ~BitEntireWr) != TypeRule)  GETDWORD( ,iAddr, iAddr+FlagLen);

	//we have (100%) node of type rule for lemmatization - now it's straight forward to lemmatize
	//read out rule
	iTmpAddr = iAddr + FlagLen;
	unsigned char iFromLen = abData[iTmpAddr];
	iTmpAddr += LenSpecLen;
	unsigned char iToLen = abData[iTmpAddr];
	iTmpAddr += LenSpecLen;

	//prepare output buffer
	unsigned char iStemLen = bWordLen - iFromLen;
	char *acReturn = new char[iStemLen + iToLen + 1];

	//do actual lematiration using given rule
	memcpy(acReturn, acWord, iStemLen);
	memcpy(&acReturn[iStemLen], &abData[iTmpAddr], iToLen);
	acReturn[iStemLen + iToLen] = 0;

	return acReturn;
}

//-------------------------------------------------------------------------------------------
//returns string representation of this node and all its subnodes
void RdrLemmatizer::ToString(ostream &os, dword iStartAddr, int iDepth, char *acParSufx, char *acParDev, char cNewChar) const {
	int iAddr = iStartAddr;
	dword *iSubs = NULL;
	unsigned char *bSubs = NULL;
	int iSubsNum = 0;
	char *acSufx = NULL;
	char *acSufxDev = NULL;

	//node type
	GETBYTEMOVE(int, iType, FlagLen);

	char *acTypeName;
	switch (iType) {
		case TypeRule:     acTypeName="RULE"; break;
		case TypeRuleEw:   acTypeName="RULE(entireword)"; break;
		case TypeLeafAC:   acTypeName="LEAF"; break;
		case TypeLeafACEw: acTypeName="LEAF(entireword)"; break;
		case TypeIntr:     acTypeName="INTER-SHORT"; break;
		case TypeIntrAC:   acTypeName="INTER-LONG"; break;
	}

	os << setfill('\t') << setw(iDepth) << "" << "" << acTypeName << ":[Addr:" << iStartAddr << "]";

	//we have rule to display
	if((iType & ~BitEntireWr)==TypeRule) {
		GETBYTEMOVE(int, iFromLen, LenSpecLen);
		GETBYTEMOVE(int, iToLen, LenSpecLen);
		GETSTRINGMOVE(char*, acTo, iToLen);
		os << "[From:" << iFromLen << "][To:" << iToLen << ",\"" << acTo << "\"]";
	}

	//we have node to display
	else {
		//eat up rule addres
		GETDWORDMOVE(dword, iRuleAddress, AddrLen);

		//eat up characters
		int iNewSufxLen = 0;
		char* acNewSuffix;
		if ((iType & BitAddChar) == BitAddChar) {
			GETBYTEMOVE(, iNewSufxLen, LenSpecLen);
			GETSTRINGMOVE(, acNewSuffix, iNewSufxLen);
		}

		//create and display sufixes
		if (cNewChar != 0) {
			int iSufxLen = 1 + iNewSufxLen + strlen(acParSufx) + 1;
			int iSufxDevLen = 2 + iNewSufxLen + strlen(acParDev) + 1;

			acSufx = new char[iSufxLen];
			acSufxDev = new char[iSufxDevLen];

			acSufxDev[0]='|';

			strncpy(&acSufx[0],acNewSuffix,iNewSufxLen);
			strncpy(&acSufxDev[1],acNewSuffix,iNewSufxLen);

			acSufx[0 + iNewSufxLen] = cNewChar;
			acSufxDev[1 + iNewSufxLen] = cNewChar;

			strcpy(&acSufx[1 + iNewSufxLen],acParSufx);
			strcpy(&acSufxDev[2 + iNewSufxLen],acParDev);

		} else {
			acSufx = "";
			acSufxDev = "|";
		}
		os << "[Suffix:" << acSufxDev << ",\"" << acSufx << "\"]";

		//display rule that applies to this node
		os << " ";
		ToString(os, iRuleAddress, 0, acSufx);

		//display hash table
		if ((iType & BitInternal) == BitInternal) {

			GETBYTEMOVE(, iSubsNum, ModLen);
			iSubs = new dword[iSubsNum];
			bSubs = new unsigned char[iSubsNum];

			ostringstream ossLine1;
			ostringstream ossLine2;
			ostringstream ossLine3;

			int iEmptyEntryNum = 0;
			for (int i = 0; i< iSubsNum; i++) {
				GETBYTEMOVE(int, cSubChar, CharLen);
				GETDWORDMOVE(dword, iSubAddres, AddrLen);
				bSubs[i]=cSubChar;
				iSubs[i]=iSubAddres;

				if (iSubAddres==0) {
					ossLine1 << " | NULL";
					ossLine2 << right << " |" << setw(5) << (int) i;
					ossLine3 << " | NULL";
					iEmptyEntryNum++;

				} else {
					ossLine1 << right << " |" << setw(3) << (char) cSubChar << "="  << setw(3) << (int) cSubChar;
					ossLine2 << right << " |" << setw(7) << (int) i;
					ossLine3 << right << " |" << setw(7) << iSubAddres;
				}
			}

			os << " HASHTABLE:";
			os << "[Size/Divider:" << iSubsNum << "]";
			os << "[Entries:" << iSubsNum - iEmptyEntryNum << "]";
			os << "[Unused:" << setprecision(4) << (float) 100 *iEmptyEntryNum/iSubsNum << "%]";
			os << " ";

			os << endl << setfill('\t') << setw(iDepth+1) << "" << "." << setfill('-') << setw(ossLine3.str().length() + 8) << ".";
			os << endl << setfill('\t') << setw(iDepth+1) << "" << "|  Pos:" << ossLine2.str() << " |";
			os << endl << setfill('\t') << setw(iDepth+1) << "" << "| Char:" << ossLine1.str() << " |";
			os << endl << setfill('\t') << setw(iDepth+1) << "" << "| Addr:" << ossLine3.str() << " |";
			os << endl << setfill('\t') << setw(iDepth+1) << "" << "'" << setfill('-') << setw(ossLine3.str().length() + 8) << "'";
			os << endl;
		}
	}

	//display sub nodes
	for (int i = 0; i<iSubsNum; i++)
		if (iSubs[i]!=0) {
			ToString(os, iSubs[i], iDepth + 1, acSufx, acSufxDev, bSubs[i]);
			if (i<iSubsNum-1) os << endl;
		}

	os.flush();
}


//-------------------------------------------------------------------------------------------
//returns nice valid c++ code representation memory block
void RdrLemmatizer::ToStringHex(ostream &os) const {
	os << dec << noshowbase;
	os << "#define RrdLemmData" << endl;
	os << "#define DATA_LEN " << iDataLen << endl;

	os << hex << right << setfill('0');
	os << "#define DATA_TBL {";

	int iLen = iDataLen/8;
	for(int i=0; i<iLen; i++) {
		if (i%5!=0) os << " ";
		else os << " \\" << endl << "\t";

		os << "0x" << setw(16) << ((qword*)abData)[i];

		if (i != iLen-1) os << ",";
	}

	os << " \\" << endl << "\t}" << endl;

	os.flush();
}

//-------------------------------------------------------------------------------------------
//loads all data needed from binary file
void RdrLemmatizer::LoadBinaryStream(istream &is) {
	iDataLen =0;
	is.read((char*) &iDataLen, 4);
	abData = new unsigned char[iDataLen];
	is.read((char*) abData, iDataLen);
}

//loads all data needed from binary file
void RdrLemmatizer::LoadBinary(const char *acFileName) {
	ifstream ifs(acFileName, ios_base::in | ios_base::binary);
	if(!ifs.good()){
		throw std::invalid_argument("Invalid model file or path.");
	}
	LoadBinaryStream(ifs);
	ifs.close();
}
