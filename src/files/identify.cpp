#include "identify.h"

#include <iostream>
#include <fstream>

int MAX(int x, int y)
{
	if((x) > (y)) 
		return x;
	else    
		return y;
}

qulonglong calculate(std::ifstream& f) {
	qulonglong hash, fsize;

	f.seekg(0, std::ios::end);
	fsize = f.tellg();
	f.seekg(0, std::ios::beg);

	hash = fsize;
	for(qulonglong tmp = 0, i = 0; i < 65536/sizeof(tmp) && f.read((char*)&tmp, sizeof(tmp)); i++, hash += tmp);
	f.seekg(MAX(0, (qulonglong)fsize - 65536), std::ios::beg);
	for(qulonglong tmp = 0, i = 0; i < 65536/sizeof(tmp) && f.read((char*)&tmp, sizeof(tmp)); i++, hash += tmp);

	return hash;	
}

QString computeHash(const QString& path)
{
	std::ifstream f;

	f.open(path.toStdString().c_str(), std::ios::in|std::ios::binary|std::ios::ate);
	if (!f.is_open()) {
		return QString();
	}
	qulonglong hash = calculate(f);
	f.close();

	//Create 16 length string of 0's and then fill from the back with hex converted bytes...
	//std::cout << setw(16) << setfill('0') << std::hex << hash;

	return "";
}
