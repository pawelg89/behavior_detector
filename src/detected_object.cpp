#include "..\includes\detected_object.h"


detected_object::detected_object(void)
{
	human = false;
	border = false;
	prediction_life_time = 0;
	KFilter = new Kalman_Filter();
    history_counter = 0;
	outside_counter = 0;
	inside_counter = 0;
	is_inside_restricted_area = false;
	//DataBase* db = new DataBase("150.254.2.23","root","admin","test");
	//if(db->Connect())
	//{
	//	bFilter = db->DescriptorLoad(1);
	//	db->Disconnect();
	//}
	//delete db;
	is_close_to = false;
	//bFilter.push_back(new BehaviorFilter("helpDescr13.dat"));
	//bFilter[bFilter.size()-1]->SetThreshold(0.2);
	//bFilter.push_back(new BehaviorFilter("helpDescr23.dat"));
	//bFilter[bFilter.size()-1]->SetThreshold(0.2);

	//bFilter.push_back(new BehaviorFilter("faintDescr0.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr1.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr2.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr3.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr4.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr5.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr6.dat"));
	//bFilter.push_back(new BehaviorFilter("faintDescr7.dat"));

	//bFilter.push_back(new BehaviorFilter("fallDescr0.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr1.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr2.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr3.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr4.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr5.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr6.dat"));
	//bFilter.push_back(new BehaviorFilter("fallDescr7.dat"));

	bFilter.push_back(new BehaviorFilter("painDescr0.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.15);
	bFilter.push_back(new BehaviorFilter("painDescr1.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.14);
	bFilter.push_back(new BehaviorFilter("painDescr2.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.15);
	bFilter.push_back(new BehaviorFilter("painDescr3.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.15);

	bFilter.push_back(new BehaviorFilter("kneelDescr0.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.2);
	bFilter.push_back(new BehaviorFilter("kneelDescr1.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.2);
	bFilter.push_back(new BehaviorFilter("kneelDescr2.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.2);
	bFilter.push_back(new BehaviorFilter("kneelDescr3.dat"));
	bFilter[bFilter.size()-1]->SetThreshold(0.2);


	//bFilter.push_back(new BehaviorFilter("faintDescr2_art.dat"));
	//bFilter[bFilter.size()-1]->SetThreshold(0.25);
	//bFilter.push_back(new BehaviorFilter("faintDescr5.dat"));
	//bFilter[bFilter.size()-1]->SetThreshold(0.25);

	//bFilter.push_back(new BehaviorFilter("fightDescr1.dat"));
	//bFilter[bFilter.size()-1]->SetThreshold(0.1);
	//bFilter.push_back(new BehaviorFilter("fightDescr4.dat"));
	//bFilter[bFilter.size()-1]->SetThreshold(0.1);

	eventSaved.resize(bFilter.size());
}

detected_object::detected_object(detected_object *obj)
{	
	number = -1;
	human = false;
	prediction_state = false;
	prediction_life_time = 0;
	KFilter = new Kalman_Filter();
	this->KFilter->KF = obj->KFilter->KF;
	is_close_to = false;
	this->bFilter = obj->bFilter;
	this->eventSaved = obj->eventSaved;
	/*
	pos_x.resize(50);
	pos_z.resize(50);*/


//	time = 0;
	last_x = 0;
	last_z = 0;
	speed = 0;
}

detected_object::~detected_object(void)
{
	delete KFilter; 
    for(size_t i = 0; i < marged_objects.size(); i++)
    { 
        delete marged_objects[i]; 
        marged_objects.erase(marged_objects.begin() + i); 
    }

	for(size_t i=0; i < bFilter.size(); i++)
	{
		delete bFilter[i];
	}
	bFilter.clear();
}

void detected_object::calc_speed(double x, double z, int timex)
{

	int history_time = 10;

	double distance;
	
	if(history_counter < history_time)
	{   /*if(time.empty())
		{
			time.push_back(timex);
		}*/
		pos_x.push_back(x);
		pos_z.push_back(z);
		time.push_back(timex);
		double x_accu = 0;
		double z_accu = 0;
		for(int i = 1; i < history_counter; i++)
		{
			x_accu+=pos_x[i] - pos_x[i-1];
			z_accu+=pos_z[i] - pos_z[i-1];
		}

		distance = sqrt(pow((x_accu),2)+pow((z_accu),2));
		history_counter++;
		speed = 0;
		if(history_counter > 1)
		speed = distance/((timex-time[0])*0.04);
	}
	else
	{
		pos_x.erase(pos_x.begin());
		pos_z.erase(pos_z.begin());
		time.erase(time.begin());
		pos_x.push_back(x);
		pos_z.push_back(z);
		time.push_back(timex);

		double x_accu = 0;
		double z_accu = 0;

		for(int i = 1; i < history_time; i++)
		{
			x_accu+=pos_x[i] - pos_x[i-1];
			z_accu+=pos_z[i] - pos_z[i-1];
		}
		distance = sqrt(pow((x_accu),2)+pow((z_accu),2));
		speed = (distance/((timex-time[0])*0.04)+speed)/2;
	}

}

void detected_object::CheckBehavior()
{
	string tmp_message;
	vector<bool> foundBehaviors;
	foundBehaviors.resize(10);
	for(size_t i=0; i<bFilter.size(); i++)
	{
		switch(bFilter[i]->behaviorType)
		{
			case 2://Calling for help
				this->bFilter[i]->Check(behDescr, is_moving);
				break;
			case 3://Fainted person
				bFilter[i]->Check(behDescr,is_moving);
				break;
			case 4://Fight
				this->bFilter[i]->Check(behDescr,is_moving);
				break;
			default:
				this->bFilter[i]->Check(behDescr,is_moving);
				;
		}
		if(bFilter[i]->found)
			foundBehaviors[bFilter[i]->behaviorType] = true;
	}

	for(int i=0; i<(int)foundBehaviors.size(); i++)
	{
		if(foundBehaviors[i])
			switch(i)
			{
				case 1:
					tmp_message.append("INTRUZ ");
					break;
				case 2:
					tmp_message.append("POMOCY ");
					break;
				case 3:
					tmp_message.append("OMDLENIE ");
					break;
				case 4:
					tmp_message.append("BOJKA ");
					break;
				case 5:
					tmp_message.append("UPADEK ");
					break;
				case 6:
					tmp_message.append("CIERPIENIE ");
					break;
				case 7:
					tmp_message.append("KUCANIE ");
					break;
				case 8:
					tmp_message.append("IDZIE ");
					break;
				case 9:
					tmp_message.append("STOI ");
					break;
				case 10:
					tmp_message.append("BIEGNIE ");
					break;
				default:
					tmp_message.append("Nieznany typ zachowania");
			}
	}
	message = tmp_message;
}

void detected_object::SetObjNumber()
{
	for(size_t i = 0; i < bFilter.size(); i++)
		bFilter[i]->objNumber = this->number;
}

vector<int> detected_object::DetectedBehaviors()
{
	vector<int> result;
	for(size_t i=0; i<bFilter.size(); i++)
	{
		if(bFilter[i]->found)
		{
			result.push_back(bFilter[i]->behaviorType);
		}
	}
	return result;
}

vector<bool> detected_object::IsFound()
{
	vector<bool> answer;
	answer.resize(bFilter.size());
	for(size_t i=0; i<bFilter.size(); i++)
		answer[i] = this->bFilter[i]->found;
	return answer;
}

bool detected_object::detect_movement()
{
	//metoda bazujaca na bledach predykcji
	//double mean_delta_x = 0;
	//double mean_delta_y = 0;
	//double counter = 0;
	//int history = 50;
	//
	//if(KFilter->kalmanv.size() < history)
	//{
	//	for(int i = 1; i < KFilter->kalmanv.size(); i++)
	//	{
	//		mean_delta_x += (KFilter->kalmanv[i].x - KFilter->kalmanv[i-1].x);
	//		mean_delta_y += (KFilter->kalmanv[i].y - KFilter->kalmanv[i-1].y);
	//		counter++;
	//	}
	//}
	//else
	//{
	//	for(int i = KFilter->kalmanv.size()-history; i < KFilter->kalmanv.size(); i++)
	//	{
	//		mean_delta_x += (KFilter->kalmanv[i].x - KFilter->kalmanv[i-1].x);
	//		mean_delta_y += (KFilter->kalmanv[i].y - KFilter->kalmanv[i-1].y);
	//		counter++;
	//	}
	//}

	//mean_delta_x = mean_delta_x/counter;
	//mean_delta_y = mean_delta_y/counter;
	//return cvPoint(KFilter->posv[KFilter->posv.size()-1].x + mean_delta_x, KFilter->posv[KFilter->posv.size()-1].y + mean_delta_y);






	////exponential smoothing
	//double alpha = 0.2; //smoothing factor
	//double state1_x = KFilter->posv[0].x;
	//double state1_y = KFilter->posv[0].y;

	//
	//for(int i = 1; i < KFilter->posv.size(); i++)
	//{
	//	state1_x = alpha * KFilter->posv[i].x + (1-alpha)*state1_x;
	//	state1_y = alpha * KFilter->posv[i].y + (1-alpha)*state1_y;
	//}

	//return cvPoint(state1_x,state1_y);



	//metoda bazujaca na wektorach
	//int history = 50;
	//double counter = 0;
	//double mean_delta_x = 0;
	//double mean_delta_y = 0;
	//if(KFilter->kalmanv.size() < history)
	//{
	//	for(int i = 1; i < KFilter->kalmanv.size(); i++)
	//	{
	//		mean_delta_x += (KFilter->kalmanv[0].x - KFilter->kalmanv[i].x);
	//		mean_delta_y += (KFilter->kalmanv[0].y - KFilter->kalmanv[i].y);
	//		counter++;
	//	}
	//}
	//else
	//{
	//	for(int i = KFilter->kalmanv.size()-history+1; i < KFilter->kalmanv.size(); i++)
	//	{	
	//		counter++;
	//	/*	mean_delta_x += 
	//		mean_delta_y += */
	//		
	//	}
	//}

	//mean_delta_x = mean_delta_x/counter;
	//mean_delta_y = mean_delta_y/counter;
	//return cvPoint(KFilter->posv[KFilter->posv.size()-1].x + mean_delta_x, KFilter->posv[KFilter->posv.size()-1].y + mean_delta_y);

  size_t movement_history = 25;
	if(KFilter->kalmanv.size() > movement_history)
	{
		for(size_t i = KFilter->kalmanv.size() - movement_history; i < KFilter->kalmanv.size(); i++) if(!isinside2(i)) return true;
		return false;
	}
	else
	{
		for(size_t i = 0; i < KFilter->kalmanv.size(); i++) if(!isinside2(i)) return true;
		return false;
	}
return false;

}

inline bool detected_object::isinside(int i)
{
	if((KFilter->kalmanv[i].x < rect.x + rect.width) && (KFilter->kalmanv[i].x > rect.x) && (KFilter->kalmanv[i].y > rect.y) && (KFilter->kalmanv[i].y < rect.y + rect.height))
	{
		return true;
	}
	else 
		return false;
}

inline bool detected_object::isinside2(int i)
{
	Rect temp_rect;

	temp_rect.x = rect.x;
	temp_rect.y = rect.y + rect.height/2;
	temp_rect.width = rect.width;
	temp_rect.height = rect.height;

	return KFilter->kalmanv[i].inside(temp_rect);

}

void detected_object::estimate_direction()
{
	int vector_history = 50;
	int mean_members = 10;
	if(static_cast<int>(KFilter->kalmanv.size()) > vector_history)
	{	
		double x1 = 0;
		double x2 = 0;
		double y1 = 0;
		double y2 = 0;
		for(size_t i = KFilter->kalmanv.size() - vector_history; i <KFilter->kalmanv.size() - vector_history + mean_members ; i++)
		{
			x1+=KFilter->kalmanv[i].x;
			y1+=KFilter->kalmanv[i].y;
		}
		for(size_t i = KFilter->kalmanv.size() - mean_members; i < KFilter->kalmanv.size(); i++)
		{
			x2+=KFilter->kalmanv[i].x;
			y2+=KFilter->kalmanv[i].y;
		}
		x1 = x1/mean_members;
		x2 = x2/mean_members;
		y1 = y1/mean_members;
		y2 = y2/mean_members;

		x_movement = (x2-x1)/(vector_history - mean_members);
		y_movement = (y2-y1)/(vector_history - mean_members);	
	}
	else 
	{	
		if(KFilter->kalmanv.size() > 2)
		{
			x_movement = (KFilter->kalmanv[KFilter->kalmanv.size()-1].x - KFilter->kalmanv[0].x)/KFilter->kalmanv.size();
			y_movement = (KFilter->kalmanv[KFilter->kalmanv.size()-1].y - KFilter->kalmanv[0].y)/KFilter->kalmanv.size();
		}
	}

}

void detected_object::ShowBehaviorStates(Mat frame)
{
	char temp_sttNum[200] = {""};
	ostrstream zapis(temp_sttNum, (int) sizeof(temp_sttNum), ios::app); 
	for(size_t i=0; i < bFilter.size(); i++)
	{
		zapis<<" | "<<bFilter[i]->GetCurrentStateNumber();
	}
	zapis<<ends;
	putText(frame,temp_sttNum,Point(this->rect.x+1,this->rect.y+10),0,0.3,Scalar(255,255,0));
}