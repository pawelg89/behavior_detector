#include "..\..\stdafx.h"
#include "..\includes\BehaviorState.h"


template <class type> bool load_data(string data_file, string data_name, type &data)
{	
	bool success = false;
	char* temp_string = new char[50];
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
	delete temp_string;

	return success;
}

//-------------------- C O N S T R U C T O R S --------------------------------------------------
BehaviorState::BehaviorState(double thresh, int method, int lPkt)
{
	lastState = false;

	idleCounter = 0;
	threshold = thresh;
	behType = 0;
	if(!load_data("parameters.txt","faintCounter",faintCounter))
		faintCounter = 80;
	if(!load_data("parameters.txt","statsON",statsON))
		statsON=false;
	//switch(method)
	//{
	//case 0:
	//	methodIdxStart = 1;
	//	methodIdxStop = 5;
	//	break;
	//case 1:
	//	methodIdxStart = 6;
	//	methodIdxStop = 10;
	//	break;
	//case 2:
	//	methodIdxStart = 11;
	//	methodIdxStop = 10+lPkt+1;
	//	break;
	//case 3:
	//	methodIdxStart = 11+lPkt+2;
	//	methodIdxStop = 10+lPkt;
	//	break;
	//default:
	//	methodIdxStart = 1;
	//	methodIdxStop = 5;
	//}
}

BehaviorState::BehaviorState(vector<PointNorm> accInput, bool isLast, double thresh, int method, int lPkt)
{
	acceptableInput = accInput;
	lastState = isLast;

	threshold = thresh;
	idleCounter = 0;
	behType = 0;
	if(!load_data("parameters.txt","faintCounter",faintCounter))
		faintCounter = 80;
	if(!load_data("parameters.txt","statsON",statsON))
		statsON=false;
	switch(method)
	{
	case 0:
		methodIdxStart = 1;
		methodIdxStop = 5;
		break;
	case 1:
		methodIdxStart = 6;
		methodIdxStop = 10;
		break;
	case 2:
		methodIdxStart = 11;
		methodIdxStop = 10+lPkt+1;
		break;
	case 3:
		methodIdxStart = 11+lPkt+2;
		methodIdxStop = (int)acceptableInput.size();
		break;
	default:
		methodIdxStart = 1;
		methodIdxStop = 5;
	}
}

BehaviorState::BehaviorState(vector<PointNorm> accInput, vector<BehaviorState*> nextstts, bool isLast, double thresh, int method, int lPkt)
{
	acceptableInput = accInput;
	nextStates = nextstts;
	lastState = isLast;

	threshold = thresh;
	idleCounter = 0;
	behType = 0;
	if(!load_data("parameters.txt","faintCounter",faintCounter))
		faintCounter = 80;
	if(!load_data("parameters.txt","statsON",statsON))
		statsON=false;
	switch(method)
	{
	case 0:
		methodIdxStart = 1;
		methodIdxStop = 5;
		break;
	case 1:
		methodIdxStart = 6;
		methodIdxStop = 10;
		break;
	case 2:
		methodIdxStart = 11;
		methodIdxStop = 10+lPkt+1;
		break;
	case 3:
		methodIdxStart = 11+lPkt+2;
		methodIdxStop = (int)acceptableInput.size();
		break;
	default:
		methodIdxStart = 1;
		methodIdxStop = 5;
	}
}

BehaviorState::BehaviorState(vector<PointNorm> accInput, vector<BehaviorState*> nextstts, bool isLast, string sttDescr, double thresh, int method, int lPkt)
{
	acceptableInput = accInput;
	nextStates = nextstts;
	lastState = isLast;
	stateDescription = sttDescr;

	threshold = thresh;
	idleCounter = 0;
	behType = 0;
	if(!load_data("parameters.txt","faintCounter",faintCounter))
		faintCounter = 80;
	if(!load_data("parameters.txt","statsON",statsON))
		statsON=false;
	switch(method)
	{
	case 0:
		methodIdxStart = 1;
		methodIdxStop = 5;
		break;
	case 1:
		methodIdxStart = 6;
		methodIdxStop = 10;
		break;
	case 2:
		methodIdxStart = 11;
		methodIdxStop = 11+lPkt+1;
		break;
	case 3:
		methodIdxStart = 11+lPkt+2;
		methodIdxStop = (int)acceptableInput.size();
		break;
	default:
		methodIdxStart = 1;
		methodIdxStop = 5;
	}
}

BehaviorState::~BehaviorState(void)
{
	acceptableInput.clear();
	nextStates.clear();
}
//-----------------------------------------------------------------------------------------------

int BehaviorState::GetConnectionsNumber()
{
	int connNum = 0;

	for(int i=0; i<(int)this->nextStates.size(); i++)
	{
		connNum += nextStates[i]->GetConnectionsNumber();
	}

	return connNum + (int)this->nextStates.size();
}

void BehaviorState::MarkConnections(bool* map, int mapSize)
{
	for(int i=0; i<(int)this->nextStates.size(); i++)
	{
		map[this->sttNumber*mapSize + this->nextStates[i]->sttNumber] = true;
		this->nextStates[i]->MarkConnections(map,mapSize);
	}
}

void BehaviorState::SaveDescriptor(ostream &output)
{
	for(int i=0; i<(int)this->acceptableInput.size(); i++)
	{
		output.write((char*)&this->acceptableInput[i].x,sizeof(double));
		output.write((char*)&this->acceptableInput[i].y,sizeof(double));
	}	
	for(int i=0; i<(int)this->nextStates.size(); i++)
	{
		this->nextStates[i]->SaveDescriptor(output);
	}
}

void BehaviorState::SaveStatistic(vector<PointNorm> inputVector)
{
	char filename_buff[100] = "";
	ostrstream zapis(filename_buff, (int) sizeof(filename_buff), ios::app);
	CreateDirectoryA((LPCSTR)"Statistics", NULL);

	for(int ptNr=0; ptNr < (int)inputVector.size(); ptNr++)
	{
		//Zapisanie wspolrzednej x
		zapis.seekp(0);
		zapis<<"Statistics/beh"<<this->behType<<"_sttNr"<<this->sttNumber<<"_pt"<<ptNr<<"_x.tmp"<<ends;
		fstream str(filename_buff, ios::out | ios::app);
		str<<inputVector[ptNr].x<<" ";
		str.close();

		//Zapisanie wspolrzednej y
		zapis.seekp(0);
		zapis<<"Statistics/beh"<<this->behType<<"_sttNr"<<this->sttNumber<<"_pt"<<ptNr<<"_y.tmp"<<ends;
		str.open(filename_buff, ios::out | ios::app);
		str<<inputVector[ptNr].y<<" ";
		str.close();
	}
}

//-------------------------- P U B L I C ---------------------------------------------------------

//BehaviorState* BehaviorState::ChangeState(vector<PointNorm> inputVector)
//{
//	if(statsON)
//		SaveStatistic(inputVector);
//	bool accepted = false;
//	//Search best possible next state
//	int chosenState = 0;
//	double bestDistance = numeric_limits<double>::max();
//	for(int i=0; i<(int)nextStates.size();i++)
//	{
//		double tempDistance = 0.0;
//		int mistakes = 1;
//		for(int j=1; j<(int)nextStates[i]->acceptableInput.size(); j++)
//		{
//			tempDistance += GetDist(inputVector[j], nextStates[i]->acceptableInput[j]);
//			if(GetDist(inputVector[j], nextStates[i]->acceptableInput[j]) <= nextStates[i]->threshold)
//			{
//				accepted = true;
//				if((j == (int)nextStates[i]->acceptableInput.size()-1) && (tempDistance < bestDistance))
//				{
//					chosenState = i;
//					bestDistance = tempDistance;
//				}				
//			}
//			else if(mistakes > 0)
//				mistakes--;
//			else
//			{
//				accepted = false;
//				break;	
//			}
//		}
//	}
//	//If possible move to that state
//	if(accepted)
//	{
//		if(nextStates[chosenState]->lastState && behType == 3)
//		{
//			if(faintCounter <= 1)
//			{
//				idleCounter = 0;
//				return this->nextStates[chosenState];
//			}
//			else
//				faintCounter--;
//		}
//		else
//		{
//			idleCounter = 0;
//			return this->nextStates[chosenState];
//		}
//	}
//	//If couldn't change state, check idle counter
//	if(idleCounter >= 90)
//	{
//		cout<<"Idle counter filled. Returning to the first state. Behavior: "<<behType<<endl;
//		idleCounter = 0;
//		return NULL;
//	}
//	else
//	{
//		if(this->sttNumber >= 1)
//			idleCounter++;
//		return this;
//	}
//}

BehaviorState* BehaviorState::ChangeState(vector<PointNorm> inputVector, bool isMoving)
{
	if(statsON)
		SaveStatistic(inputVector);
	bool accepted = false;
	//Search best possible next state
	int chosenState = 0;
	double bestDistance = numeric_limits<double>::max();
	for(int i=0; i<(int)nextStates.size();i++)
	{
		double tempDistance = 0.0;
		int mistakes;
		switch(methodIdxStart)
		{
		case 11:			
			if(behType == 6 || behType == 7)
				mistakes = (int)(0.15 * (nextStates[i]->methodIdxStop-nextStates[i]->methodIdxStart)+0.5);
			else
				mistakes = (int)(0.35 * (nextStates[i]->methodIdxStop-nextStates[i]->methodIdxStart)+0.5);
			break;
		case 6:
			if(behType==6 || behType==7) 
				mistakes = 0;
			else 
				mistakes = 1;
			break;
		case 1:
			if(behType==6 || behType==7) mistakes = 0;
			else mistakes = 1;
			break;
		default:
			mistakes = (int)(0.12 * (nextStates[i]->methodIdxStop-nextStates[i]->methodIdxStart)+0.5);
			break;
		}
		//Debug
		//int pt_match_count = 0;

		for(int j=nextStates[i]->methodIdxStart; j<nextStates[i]->methodIdxStop && j<(int)inputVector.size() && j<(int)nextStates[i]->acceptableInput.size(); j++)
		{
			tempDistance += GetDist(inputVector[j], nextStates[i]->acceptableInput[j]);
			if( (GetDist(inputVector[j], nextStates[i]->acceptableInput[j]) <= nextStates[i]->threshold) || (nextStates[i]->methodIdxStart>11 && tempDistance <= (nextStates[i]->methodIdxStop-nextStates[i]->methodIdxStart)*nextStates[i]->threshold) )
			{
				//pt_match_count++;
				accepted = true;
				if((j == methodIdxStop-1) && (tempDistance < bestDistance))
				{
					chosenState = i;
					bestDistance = tempDistance;					
				}
			}
			else if(mistakes > 0)
				mistakes--;
			else
			{
				accepted = false;
				break;
			}
		}
	}
	//If possible move to that state
	if(accepted)
	{
		if(nextStates[chosenState]->sttNumber == 10)
		{
			cout<<endl;
		}
		if(nextStates[chosenState]->lastState == true && behType == 3)
		{
			if(!isMoving)//Fainted people don't move... usually
			{
				if(faintCounter <= 1)
				{
					idleCounter = 0;
					return this->nextStates[chosenState];
				}
				else
					faintCounter--;
			}
		}
		else
		{
			idleCounter = 0;
			return this->nextStates[chosenState];
		}
	}
	if(idleCounter >= 70)
	{
		cout<<"Idle counter filled. Returning to the first state. Behavior: "<<behType<<endl;
		idleCounter = 0;
		return NULL;
	}
	else
	{
		if(this->sttNumber >= 1)
		{
			idleCounter++;
			//cout<<"State: "<<this->sttNumber<<" has idleCounter: "<<idleCounter<<endl;
		}
		return this;
	}
}

//BehaviorState* BehaviorState::ChangeState(vector<PointNorm> inputVector/*, vector<Rect> vRect*/)
//{
//	bool accepted = false;
//	int mistakes = 0;
//
//	for(int i=0; i<acceptableInput.size(); i++)
//	{
//		for(int j=1; j<acceptableInput[i].size(); j++)
//		{
//			if(GetDist(inputVector[j],acceptableInput[i][j]) > threshold)
//			{
//				if(mistakes>0)
//					mistakes--;
//				else
//				{
//					idleCounter++;
//					i = acceptableInput.size();
//					accepted = false;
//					break;
//				}
//			}
//			else
//			{
//				accepted = true;
//				//nextStates[i]->rectNumber = j;
//			}
//
//		}
//		if(accepted)
//		{
//			idleCounter = 0;
//			return this->nextStates[i];
//		}
//	}
//	if(idleCounter >= 100000)
//		return NULL;
//	else
//		return this;
//	
//}

inline double BehaviorState::GetDist(PointNorm A, PointNorm B)
{
	return sqrt( (A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y));
}

void BehaviorState::SetNextStates(vector<BehaviorState*> nextStts)
{
	nextStates = nextStts;
}

void BehaviorState::SetIdxStop(int idxStop)
{
	this->methodIdxStop = idxStop;
}