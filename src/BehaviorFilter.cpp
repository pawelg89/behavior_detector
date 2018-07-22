#include "..\includes\BehaviorFilter.h"
#include <fstream>

template <class type> bool load_data(string data_file, string data_name, type &data)
{	
	bool success = false;
	std::string temp_string = "";
	ifstream input;
	input.open(data_file);
	if(!input.is_open())
		return success;

	while(!input.eof())
	{
		input >> temp_string;
		if(temp_string == data_name)
		{
			input >> data;
			success = true;
			break;
		}
		else input >> temp_string;
	}
	input.close();

	return success;
}
int BF_LOG(string msg, int level, int log_level = 1, bool newLine = true) 
{ 
    ofstream log; 
    if(level <= log_level) 
    { 
        time_t rawtime; 
        time (&rawtime);                
        struct tm * timeinfo = localtime (&rawtime); 
            
        char filename_buff[100] = ""; 
        ostrstream zapis(filename_buff, (int) sizeof(filename_buff), ios::app); 
        zapis<<"log_"<<timeinfo->tm_year+1900<<"_"<<timeinfo->tm_mon+1<<"_"<<timeinfo->tm_mday<<".txt"<<ends; 
        if(!log.is_open()) 
            log.open(filename_buff, ios_base::app); 
        if(newLine) 
        { 
            log<<"\n["<< setw(2) << setfill('0') <<timeinfo->tm_hour;
			log<<":"<< setw(2) << setfill('0') <<timeinfo->tm_min;
			log<<":"<< setw(2) << setfill('0') <<timeinfo->tm_sec;
            log<<"] "<<"BehaviorFilter: "<<msg<<flush; 
        } 
        else 
        { 
            log<<msg<<flush; 
        } 
        //Zamykamy wektor i zawijamy impreze 
        log.close(); 
        return 0; 
    } 
    else 
    { 
        return 1; 
    } 
}

BehaviorFilter::BehaviorFilter(void)
{
}

BehaviorFilter::BehaviorFilter(char* path)
{
	method.resize(11,0);
	if(!load_data("parameters.txt","lPkt", lPkt))
		lPkt = 10;
	if(!load_data("parameters.txt","method_help",method[2]))
		method[2] = 0;
	if(!load_data("parameters.txt","method_faint",method[3]))
		method[3] = 0;
	if(!load_data("parameters.txt","method_fight",method[4]))
		method[4] = 0;
	if(!load_data("parameters.txt","method_fall",method[5]))
		method[5] = 0;
	if(!load_data("parameters.txt","method_pain",method[6]))
		method[6] = 0;
	if(!load_data("parameters.txt","method_kneel",method[7]))
		method[7] = 0;

	//Reading descriptor from file
	ifstream input;
	input.open(path, ios_base::binary);
	if(!input.is_open())
		throw (string)"Couldn't open descriptor file. Check descriptors path: "+path;

	int header[2];//0-liczba stanow glownych, 1-typ zachowania
	input.read((char*) header, sizeof(header));
	this->behaviorType = header[1];

	//liczba punktow w stanach
	int* sizes = new int[header[0]];
	input.read((char*) sizes, sizeof(int)*header[0]);

	//Progi akceptacji wektora z punktami deskryptora
	double* thresholds = new double[header[0]];
	input.read((char*) thresholds, sizeof(double)*header[0]);

	//Mapa polaczen miedzy stanami
	int statesNumber = header[0]+1;
	bool* map = new bool[statesNumber*statesNumber];
	input.read((char*) map,sizeof(bool)*statesNumber*statesNumber);

	//Oznaczenia czy sa to ostatnie stany
	bool* islast = new bool[statesNumber];
	input.read((char*) islast,sizeof(bool)*statesNumber);

	//Liczba wszystkich punktów deskryptora, potrzebna do odczytania samego deskryptora
	int header_1 = 0;
	for(int i=0; i<header[0];i++)
		header_1+=sizes[i];
	
	//tablica z punktami (x,y) dla deskryptorow. <serialized>
	double* buffer = new double[header_1*2];
	input.read((char*) buffer, sizeof(double)*header_1*2);
	
	vector<vector<PointNorm>> behaviorDescr;
	behaviorDescr.resize(header[0]);
	int licznik_punktow = 0;
	for(int i=0; i<header[0]; i++)
	{
		for(int j=0; j<sizes[i]; j++)
		{
			behaviorDescr[i].push_back(PointNorm(buffer[licznik_punktow],buffer[licznik_punktow + 1]));
			licznik_punktow+=2;
		}
	}

	//Utworz siatke stanow (automat)
	temp_StateHandles.push_back(new BehaviorState(thresholds[0],this->method[behaviorType], this->lPkt));//first state just with a handle to next states, mostly idle state
	temp_StateHandles[0]->sttNumber = 0;
	temp_StateHandles[0]->behType = behaviorType;
	//temp_StateHandles[0]->SetIdxStop( (int)behaviorDescr[0].size() );
	for(int i=0; i<header[0];i++)
	{
		temp_StateHandles.push_back(new BehaviorState(behaviorDescr[i], islast[i+1], thresholds[i], this->method[behaviorType],this->lPkt));
		temp_StateHandles[i+1]->sttNumber = i+1;
		temp_StateHandles[i+1]->behType = behaviorType;
	}

	//Create connections
	for(int i=0; i<statesNumber; i++)
	{
		vector<BehaviorState*> temp_nxtStts;
		for(int j=0; j<statesNumber; j++)
		{
			if(map[statesNumber*i + j])
			{
				temp_nxtStts.push_back(temp_StateHandles[j]);
			}
		}
		temp_StateHandles[i]->SetNextStates(temp_nxtStts);
		temp_nxtStts.clear();
	}

	//Filter description. It shows up on blob with detected behavior
	switch(behaviorType)
	{
		case 1://Wtargniecie na obszar chroniony
			this->behaviorDescription = "INTRUZ";
			break;
		case 2://Wolanie o pomoc
			this->behaviorDescription = "POMOCY";
			break;
		case 3://Omdlenie
			this->behaviorDescription = "OMDLENIE";
			break;
		case 4://Bojka
			this->behaviorDescription = "BOJKA";
			break;
		case 5://OMDLENIE bez czekania
			this->behaviorDescription = "UPADEK";
			break;
		case 6://Nasze wymioty
			this->behaviorDescription = "CIERPIENIE";
			break;
		case 7://Wiaze buta
			this->behaviorDescription = "KUCANIE";
			break;
		case 8:
			this->behaviorDescription = "IDZIE";
			break;
		case 9:
			this->behaviorDescription = "STOI";
			break;
		case 10:
			this->behaviorDescription = "BIEGNIE";
			break;
		default:
			this->behaviorDescription = "Nieznany typ zachowania";
	}

	firstState = temp_StateHandles[0];
	currentState = firstState;
	found = false;

	//Clearing helper vector
	delete[] sizes;
	delete[] thresholds;
	delete[] map;
	delete[] islast;
	delete[] buffer;
}

BehaviorFilter::BehaviorFilter(char* path, int gs) 
{ 
    //Reading descriptor from file 
    ifstream input; 
    input.open(path, ios_base::binary); 
    int header[3];//0-liczba polaczen miedzy stanami, 1-liczba punktow dla jednej pozy 
    input.read((char*) header, sizeof(header)); 
    behaviorType = header[2]; 
    double threshold; 
    input.read((char*) &threshold, sizeof(double)); 
    vector<vector<PointNorm>> behaviorDescr; 
    behaviorDescr.resize(header[0]); 
    //Mapa polaczen miedzy stanami 
    int statesNumber = header[0]+1; 
    bool* map = new bool[statesNumber*statesNumber]; 
    input.read((char*) map,sizeof(bool)*statesNumber*statesNumber); 
    //tablica z punktami (x,y) dla deskryptorow. <serialized> 
    double* buffer = new double[header[0]*header[1]*2]; 
    input.read((char*) buffer, sizeof(double)*header[0]*header[1]*2); 
    for(int i=0; i<header[0]; i++) 
    { 
        for(int j=0; j<header[1]; j++) 
        { 
            behaviorDescr[i].push_back(PointNorm(buffer[i*2*header[1] + 2*j],buffer[i*2*header[1] + 2*j + 1])); 
        } 
    } 
    
    temp_StateHandles.push_back(new BehaviorState(threshold));//first state just with a handle to next states, mostly idle state 
    temp_StateHandles[0]->sttNumber = 0; 
    temp_StateHandles[0]->behType = behaviorType; 
    for(int i=0; i<header[0];i++) 
    { 
        if( i == (header[0]-1)) 
            temp_StateHandles.push_back(new BehaviorState(behaviorDescr[i], true, threshold)); 
        else 
            temp_StateHandles.push_back(new BehaviorState(behaviorDescr[i], false, threshold)); 
        temp_StateHandles[i+1]->sttNumber = i+1; 
        temp_StateHandles[i+1]->behType = behaviorType; 
    } 
    //Create connections 
    for(int i=0; i<statesNumber; i++) 
    { 
        vector<BehaviorState*> temp_nxtStts; 
        for(int j=0; j<statesNumber; j++) 
        { 
            if(map[statesNumber*i + j]) 
            { 
                temp_nxtStts.push_back(temp_StateHandles[j]); 
            } 
        } 
        temp_StateHandles[i]->SetNextStates(temp_nxtStts); 
        temp_nxtStts.clear(); 
    } 
    //Filter description. It shows up on blob with detected behavior 
    switch(behaviorType) 
    { 
       case 1://Wtargniecie na obszar chroniony
			this->behaviorDescription = "INTRUZ";
			break;
		case 2://Wolanie o pomoc
			this->behaviorDescription = "POMOCY";
			break;
		case 3://Omdlenie
			this->behaviorDescription = "OMDLENIE";
			break;
		case 4://Bojka
			this->behaviorDescription = "BOJKA";
			break;
		case 5://OMDLENIE bez czekania
			this->behaviorDescription = "UPADEK";
			break;
		case 6://Nasze wymioty
			this->behaviorDescription = "CIERPIENIE";
			break;
		case 7://Wiaze buta
			this->behaviorDescription = "KUCANIE";
			break;
		case 8:
			this->behaviorDescription = "IDZIE";
			break;
		case 9:
			this->behaviorDescription = "STOI";
			break;
		case 10:
			this->behaviorDescription = "BIEGNIE";
			break;
		default:
			this->behaviorDescription = "Nieznany typ zachowania";
    } 
    firstState = temp_StateHandles[0]; 
    currentState = firstState; 
    found = false; 
    delete[] map; 
    delete[] buffer; 
}

BehaviorFilter::~BehaviorFilter()
{
	for(int i=0; i<(int)temp_StateHandles.size(); i++)
	{
		delete temp_StateHandles[i];
	}
	temp_StateHandles.clear();
}
//
//void BehaviorFilter::Check(vector<PointNorm> input)
//{
//	currentState = currentState->ChangeState(input);
//	//Przypadek, gdy zbyt dlugo czekalismy na nastepny stan
//	if(currentState == NULL)
//		currentState = firstState;
//	//Jesli przeszlismy wlasnie do ostatniego to juz to wykrywamy
//	if(currentState->lastState) 
//    { 
//		char* temp_oNum;
//		sprintf(temp_oNum, ", objNum: %d.", this->objNumber);
//		this->behaviorDescription.append(temp_oNum);
//		//strcat(temp_msg, temp_oNum);
//        BF_LOG(this->behaviorDescription,1,log_level);
//    }
//
//	if( found || currentState->lastState)
//	{	
//		currentState = firstState;
//		found = true;
//	}
//	else
//		found = false;
//}

void BehaviorFilter::Check(vector<PointNorm> input, bool isMoving)
{
	currentState = currentState->ChangeState(input, isMoving);
	//Przypadek, gdy zbyt dlugo czekalismy na nastepny stan
	if(currentState == NULL)
		currentState = firstState;
	//Jesli przeszlismy wlasnie do ostatniego to juz to wykrywamy
	if(currentState->lastState) 
    { 
		char* temp_oNum = new char[100];
		sprintf(temp_oNum, ", objNum: %d.", this->objNumber);
		this->behaviorDescription.append(temp_oNum);
        BF_LOG(this->behaviorDescription,1,log_level); 
		delete []temp_oNum;
    }

	if( found || currentState->lastState)
	{	
		currentState = firstState;
		found = true;
	}
	else
		found = false;
}

vector<int> BehaviorFilter::BehaviorType()
{
	vector<int> result;
	if(found)
	{
		if(behaviorDescription == "INTRUZ")
			result.push_back(1);
		else if(behaviorDescription == "POMOCY")
			result.push_back(2);
		else if (behaviorDescription == "OMDLENIE")
			result.push_back(3);
		else if (behaviorDescription == "BOJKA")
			result.push_back(4);
		else if(behaviorDescription == "UPADEK")
			result.push_back(5);
		else if(behaviorDescription == "CIERPIENIE")
			result.push_back(6);
		else if(behaviorDescription == "KUCANIE")
			result.push_back(7);
		else if(behaviorDescription == "IDZIE")
			result.push_back(8);
		else if(behaviorDescription == "STOI")
			result.push_back(9);
		else if(behaviorDescription == "BIEGNIE")
			result.push_back(10);
	}
	return result;
}

int BehaviorFilter::GetCurrentStateNumber()
{
	return currentState->sttNumber;
}

void BehaviorFilter::Rewrite(char* path, double thresholdNew)
{
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------LOAD USING OLD DESCRIPTOR FORMAT-----------------------------------------------------------------------------------------
	//Reading descriptor from file
	ifstream input;
	input.open(path, ios_base::binary);
	int header[3];//0-liczba polaczen miedzy stanami, 1-liczba punktow dla jednej pozy
	input.read((char*) header, sizeof(header));
	//double threshold;
	//input.read((char*) &threshold, sizeof(double));

	//Mapa polaczen miedzy stanami
	int statesNumber = header[0]+1;
	bool* map = new bool[statesNumber*statesNumber];
	input.read((char*) map,sizeof(bool)*statesNumber*statesNumber);
	
	//tablica z punktami (x,y) dla deskryptorow. <serialized>
	double* buffer = new double[header[0]*header[1]*2];
	input.read((char*) buffer, sizeof(double)*header[0]*header[1]*2);
	input.close();

	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------SAVE USING NEW DESCRIPTOR FORMAT-----------------------------------------------------------------------------------------
	ofstream descriptor_file;
	descriptor_file.open(path,ios_base::binary);
	bool temp = descriptor_file.is_open();
	int size1 = header[0];//Liczba przejœæ wa¿onych wektorem wejœciowym
	int size2 = header[1];//Liczba punktow deskryptora dla kazdego stanu
	int descrType = header[2];



	descriptor_file.write((char*)&size1,4);
	descriptor_file.write((char*)&size2,4);
	descriptor_file.write((char*)&descrType,4);
	descriptor_file.write((char*) &thresholdNew,8);

	//Zapisz mape polaczen
	descriptor_file.write((char*)map, sizeof(bool)*statesNumber*statesNumber);

	//Wpisz spowrotem deskryptor
	descriptor_file.write((char*) buffer, sizeof(double)*header[0]*header[1]*2);
			
	descriptor_file.close();
	delete[] map;
	delete[] buffer;
}

void BehaviorFilter::SetFirstState(BehaviorState* first)
{
	firstState = first;
	currentState = first;
}

BehaviorState* BehaviorFilter::GetFirstState()
{
	return firstState;
}

void BehaviorFilter::Save(char* path)
{
	ofstream descriptor_file;
	descriptor_file.open(path,ios_base::binary);
	if(!descriptor_file.is_open())
		throw (string) "Couldn't open desired location. Probable access denied, check user privileges (may not work on ../ProgramFiles on NON Administrator account).";

	BehaviorState* iter = temp_StateHandles[(int)temp_StateHandles.size()-1];
	int size1 = iter->sttNumber;//Liczba stanów g³ównych -> pierwszy rozmiar acceptable input
	//int size2 = (int)iter->acceptableInput.size();//Liczba punktow deskryptora dla kazdego stanu
	
	int sttsNumber = size1+1;


	descriptor_file.write((char*)&size1,4);
	//descriptor_file.write((char*)&size2,4);
	descriptor_file.write((char*)&behaviorType,4);

	//----- Zapisanie wektora z dlugosciami deskryptorow poz ----------
	int* sizes = new int[size1];
	for(int i=1; i<sttsNumber; i++)
		sizes[i-1] = (int)temp_StateHandles[i]->acceptableInput.size();
	descriptor_file.write((char*)sizes, size1*sizeof(int));
	//-----Zapisanie wektora z progami wejsciowymi---------------------
	double* thresholds = new double[size1];
	for(int i=0; i<size1; i++)
		thresholds[i] = temp_StateHandles[i+1]->threshold;

	descriptor_file.write((char*)thresholds,sizeof(double)*size1);
	//-----------------------------------------------------------------
	//----Zapisanie mapy polaczen miedzy stanami-----------------------
	bool* map = new bool[sttsNumber*sttsNumber];
	for(int i=0; i<sttsNumber*sttsNumber;i++)
		map[i] = false;

	this->firstState->MarkConnections(map, sttsNumber);

	descriptor_file.write((char*)map, sizeof(bool)*sttsNumber*sttsNumber);
	//-----------------------------------------------------------------
	//----Zapisanie wektora z oznaczeniami stanow wyjsciowych----------
	bool* islast = new bool[sttsNumber];
	for(int i=0; i<sttsNumber; i++)
		islast[i] = temp_StateHandles[i]->lastState;

	descriptor_file.write((char*)islast, sizeof(bool)*sttsNumber);
	//-----------------------------------------------------------------
	//----Wpisz punkty deskryptora-------------------------------------
	this->firstState->SaveDescriptor(descriptor_file);
	
	descriptor_file.close();
	delete[] thresholds;
	delete[] islast;
	delete[] map;
}

void BehaviorFilter::SetThreshold(double thresh)
{
	for(size_t i=0; i<this->temp_StateHandles.size(); i++)
	{
		temp_StateHandles[i]->threshold = thresh;
	}
}

void BehaviorFilter::SetBehaviorType(int type, string message)
{
	this->behaviorType = type;
	this->behaviorDescription = message;
	for(size_t i=0; i<this->temp_StateHandles.size(); i++)
	{
		temp_StateHandles[i]->behType = type;
	}
}

void BehaviorFilter::Append(BehaviorFilter* appFilter)
{
	//Dodaj po³¹czenia do struktury
	for(int i=0; i< (int)appFilter->firstState->nextStates.size(); i++)
	{
		firstState->nextStates.push_back(appFilter->firstState->nextStates[i]);
	}
	//WskaŸniki prze³ó¿ do lokalnego wektora ze stanami
	for(int i=1; i<(int)appFilter->temp_StateHandles.size(); i++)
	{
		temp_StateHandles.push_back(appFilter->temp_StateHandles[i]);
	}
	//Aktualizuj numery stanow
	for(int i=0; i<(int)temp_StateHandles.size(); i++)
	{
		temp_StateHandles[i]->sttNumber = i;
	}
}