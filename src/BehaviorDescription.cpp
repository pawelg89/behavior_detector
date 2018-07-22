#include "..\includes\BehaviorDescription.h"
#include <fstream>

using namespace std;


BehaviorDescription::BehaviorDescription(void)
{
}


BehaviorDescription::~BehaviorDescription(void)
{
}

void BehaviorDescription::SaveBehaviorDescriptor(char* descriptor_name/*, int method*/)
{	
	int descrType = 0;
	cout<<"What type of descriptor filter is it? 2-Calling for Help, 3-Faint, 4-Fighting, 5-Fall, 6-Pain&Suffering, 7-Kneeling."<<endl;
	cin>>descrType;
	char buffer[18];

	//Numeracja deskryptorów
	ifstream num; 
	int cntr;
	switch(descrType)
	{
		case 2://Wolanie o pomoc
			num.open("handsNum.txt");
			num>>cntr;
			
			sprintf(buffer,"helpDescr%d.dat",cntr);
			num.close();
			break;
		case 3://Omdlenie
			num.open("faintNum.txt");
			num>>cntr;

			sprintf(buffer,"faintDescr%d.dat",cntr);
			num.close();
			break;
		case 4://Bojka
			num.open("fightNum.txt");
			num>>cntr;

			sprintf(buffer,"fightDescr%d.dat",cntr);
			num.close();
			break;
		case 5://Upadek czyli to samo co omdlenie tylko bez licznika
			num.open("fallNum.txt");
			num>>cntr;

			sprintf(buffer, "fallDescr%d.dat",cntr);
			num.close();
			break;
		case 6://Nasz wymioty to lapanie sie za brzuchal
			num.open("painNum.txt");
			num>>cntr;

			sprintf(buffer, "painDescr%d.dat", cntr);
			num.close();
			break;
		case 7://Kucanie
			num.open("kneelNum.txt");
			num>>cntr;

			sprintf(buffer, "kneelDescr%d.dat", cntr);
			num.close();
			break;

		default:
			cntr = 0;
	}

	ofstream descriptor_file;
	descriptor_file.open(buffer,ios_base::binary);
	bool temp = descriptor_file.is_open();
	int size1 = descriptor.size();//Liczba polaczen
	int size3 = size1+1;

	//header
	descriptor_file.write((char*)&size1,4);
	descriptor_file.write((char*)&descrType,4);
	
	//int size2 = descriptor[size1-1].size();//Liczba punktow deskryptora dla kazdego stanu
	//descriptor_file.write((char*)&size2,4);

	//Wektor z rozmiarami deskryptorów
	descriptor_file.write((char*) sizes,sizeof(int)*size1);

	//Progi dla wszystkich stanow
	double* thresholds = new double[size1];
	//cout<<"Give threshold: "<<endl;
	double temp1;	
	temp1=0.16;//cin>>temp1;
	for(int i=0; i<size1; i++)
		thresholds[i] = temp1;
	cout<<endl;

	descriptor_file.write((char*)thresholds,sizeof(double)*size1);
	
	//Mapa polaczen miedzy stanami, przy tworzeniu domyslna tylko z ³ancuchem 1->2->3-> .. ->n
	map = new bool[size3*size3];
	for(int i=0; i<size3; i++)
		for(int j=0; j<size3; j++)
			map[i*size3 + j] = ( j == i+1 );

	descriptor_file.write((char*)map, sizeof(bool)*size3*size3);

	//Oznaczenia czy sa to ostatnie stany
	bool* isLast = new bool[size3];
	for(int i=0; i<size3; i++)
	{
		isLast[i] = ((i==size1)?true:false);
	}
	descriptor_file.write((char*)isLast, sizeof(bool)*size3);

	//tablica z punktami (x,y) dla deskryptorow. <serialized>
	for(int i=0; i<size1; i++)
	{
		for(int j=0; j<sizes[i]; j++)
		{
			descriptor_file.write((char*)&descriptor[i][j].x,sizeof(double));
			descriptor_file.write((char*)&descriptor[i][j].y,sizeof(double));
		}
	}
	descriptor_file.close();

	//Update Numeracji deskryptorów w pliku
	ofstream num_plus;
	switch(descrType)
	{
		case 2://Wolanie o pomoc
			num_plus.open("handsNum.txt");
			num_plus<<(++cntr);
			num_plus.close();
			break;
		case 3://Omdlenie
			num_plus.open("faintNum.txt");
			num_plus<<(++cntr);
			num_plus.close();
			break;
		case 4://Bojka
			num_plus.open("fightNum.txt");
			num_plus<<(++cntr);
			num_plus.close();
			break;
		case 5://Upadek
			num_plus.open("fallNum.txt");
			num_plus<<(++cntr);
			num_plus.close();
			break;
		case 6://Cierpienie
			num_plus.open("painNum.txt");
			num_plus<<(++cntr);
			num_plus.close();
			break;
		case 7://Kucanie
			num_plus.open("kneelNum.txt");
			num_plus<<(++cntr);
			num_plus.close();
			break;
		default:
			cntr = 0;
	}

	delete[] thresholds;
	delete[] map;
	delete[] isLast;
}