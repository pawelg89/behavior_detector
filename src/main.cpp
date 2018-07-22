// Detektor_v101.cpp : Defines the entry point for the console application.
//

#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include "..\includes\DiffImage.h"
#include <fstream>
#include <strstream>

using namespace cv;
using namespace std;

int log_level;

int VR_LOG(string msg, int level, int log_level = 1)
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
		log<<"\n["<< setw(2) << setfill('0') <<timeinfo->tm_hour;
		log<<":"<< setw(2) << setfill('0') <<timeinfo->tm_min;
		log<<":"<< setw(2) << setfill('0') <<timeinfo->tm_sec;
        log<<"] "<<msg<<flush;

		//Zamykamy wektor i zawijamy impreze
		log.close();
		return 0;
	}
	else
	{
		return 1;
	}
}

int main(int argc, char* argv[])
{
	if(argc > 1)
	{
		ifstream file;
		file.open(argv[1],ios_base::in);
		if(!file.is_open())
		{
			cout<<"Video file does not exist."<<endl;
			return 0;
		}
		file.close();
		DiffImage* dif = new DiffImage(argv[1]);
		dif->DiffImageAction();
	}
	else
	{
		VR_LOG("-------------------------------- ENTRY POINT --------------------------------------------------",1);
		//cout<<"Too few arguments."<<endl;
		//DiffImage* dif = new DiffImage("C:/Users/Projekt_1/Pictures/LifeCam Files/kr_kuc_SL_prawo.wmv");
		//DiffImage* dif = new DiffImage("C:/Users/Gardzin/Videos/sekwencje testowe/faint_testsequence1.avi");
		//DiffImage* dif = new DiffImage("C:/Users/Gardzin/Videos/sekwencje testowe/faint_school.avi");
		//DiffImage* dif = new DiffImage("C:/Users/Gardzin/Videos/sekwencje testowe/zima.mp4");
		//DiffImage* dif = new DiffImage("C:/Users/Gardzin/Videos/sekwencje testowe/testy1.avi");
		//DiffImage* dif = new DiffImage("C:/Users/Gardzin/Videos/sekwencje testowe/krzysiu.avi");

		//DiffImage* dif = new DiffImage("C:/Users/Martyna/Videos/sekwencje testowe/krzysiu.avi");
		DiffImage* dif = new DiffImage("C:/Users/Martyna/Videos/sekwencje testowe/machanie_test.avi");
		//DiffImage* dif = new DiffImage("C:/Users/Martyna/Videos/sekwencje testowe/zima.mp4");

		try
		{
			dif->DiffImageAction2();
		}
		catch(string err_msg)
		{ 
			cout<<err_msg<<endl;
			VR_LOG(err_msg,1);
			system("pause");
		}
		catch(ios_base::failure err)
		{
			cout << "Brak pozwolenia na zapis na dysku." << endl;
			VR_LOG("Brak pozwolenia na zapis na dysku.",1);
		}		
		VR_LOG("------------------------------ PROGRAM FINISHED -----------------------------------------------",1);
	}

	//system("pause");
	return 0;
}
