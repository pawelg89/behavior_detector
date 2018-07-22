#include "..\..\stdafx.h"
#include "..\includes\GCapture.h"
#include <strstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>


using namespace cv;
using namespace std;

//---------------------------------------------------------------------------
//-----------UWAGA STREAFA ZAGROZONA, ZMIENNE GLOBALNE-----------------------
//----------------------NIE DOTYKAC!!!!--------------------------------------
//---------------------------------------------------------------------------
vector<CRITICAL_SECTION> critical;
vector<HANDLE> eventStart;
vector<Mat> buffer;
vector<Mat> fore_vec;
vector<BackgroundSubtractorMOG2*> bg_vec;

vector<VideoCapture*> video{};

unsigned int __stdcall buffer_thread(void* arg)
{
	int cameraNumber = (int) arg;
  cout << video.size() << " <--> " << cameraNumber << endl;
  while (video.size() < cameraNumber + 1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cout << "FAAK" << endl;
  }
	//VideoCapture video(0);
	//CvCapture* video = cvCaptureFromFile((char*)arg);
	//CvCapture* video = cvCaptureFromAVI("D:\\zima.mp4");
	//CvCapture* video = cvCaptureFromCAM(0);
	while(1)
	{
		while(!TryEnterCriticalSection(&critical[cameraNumber])){}
		
		video[cameraNumber]->grab();
		video[cameraNumber]->grab();
		video[cameraNumber]->retrieve(buffer[cameraNumber]);
		bg_vec[cameraNumber]->operator()(buffer[cameraNumber],fore_vec[cameraNumber]);
		SetEvent(eventStart[cameraNumber]);

		LeaveCriticalSection(&critical[cameraNumber]);
	}

	//cvReleaseCapture(&video);
	video[cameraNumber]->release();
	return 0;
}
//---------------------------------------------------------------------------
//-------------------KONIEC STREFY-------------------------------------------
//---------------------------------------------------------------------------

template <class type> bool load_data(string data_file, string data_name, type &data)
{	
	bool success = false;
	char temp_string[50];
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

	data_file.clear();
	data_name.clear();

	return success;
}

GCapture::GCapture(void)
{
}

GCapture::GCapture(int cams_number)
{
	buffer.resize(cams_number);
	fore_vec.resize(cams_number);
	critical.resize(cams_number);
	eventStart.resize(cams_number);
	for(int i=0; i<cams_number; i++)
	{
		//Ustalenie sciezek dostepu do strumieni wideo
		char pathDirX[1000] = "";
		ostrstream monkeys(pathDirX, (int) sizeof(pathDirX), ios::app);	//hehe legacy stream name ];->
		string tempDir;
		monkeys.seekp(0);
		monkeys<<"pathDir"<<i<<ends;
		load_data("parameters.txt", pathDirX, tempDir);

		//Inicjalizacja sekcji krytycznych dla poszczegolnych kamer i eventu na start mozliwosci odczytania
		InitializeCriticalSection(&critical[i]);
		eventStart[i] = CreateEvent(0,0,0,0);

		//MOGI !!!
		float mogThreshold;
		float backgroundRatio;
		if(!load_data("parameters.txt","mogThreshold",mogThreshold))
			mogThreshold = 50.0f;
		if(!load_data("parameters.txt","backgroundRatio",backgroundRatio))
		backgroundRatio = 0.7f;
		bg_vec.push_back( new BackgroundSubtractorMOG2(10000, mogThreshold,true));
//		bg_vec[i]->backgroundRatio = backgroundRatio;

		//Wczytanie strumieni do handlerow no i zapisanie tych sciezek na potem (w sumie nie wiem czy sie przyda ale moze sie przyda :P)
    cout << "Video path: " << tempDir << endl;
    video.clear();
		video.push_back(new VideoCapture(tempDir));
		//video.push_back(new VideoCapture(i) );
		pathDir.push_back(tempDir);

		//Wczytanie watkow i ich uruchomienie
		HANDLE thrdBuffer = (HANDLE)_beginthreadex(0, 0, &buffer_thread, (void*)i, 0, 0);
		watkiKamer.push_back(thrdBuffer);
		WaitForSingleObject(eventStart[i], INFINITE);
		cout<<"Camera["<<i<<"] connection established."<<endl;
	}
}

GCapture::~GCapture(void)
{
	//Usuwanie watkow
	for(int i=0; i<(int)watkiKamer.size(); i++)
	{
		TerminateThread(watkiKamer[i],0);
		CloseHandle(watkiKamer[i]);
		CloseHandle(eventStart[i]);
	}
}



void GCapture::QueryFrame(Mat &frame,Mat &fore, int i)
{
	while(!TryEnterCriticalSection(&critical[i])){}
	if(!buffer[i].empty())
	{
		buffer[i].copyTo(frame);
		fore_vec[i].copyTo(fore);
		//copy( buffer, frame );
	}
	else
    frame = Mat{};
	LeaveCriticalSection(&critical[i]);
}