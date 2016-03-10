#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

struct Wave{
	int channels;
	int sample_rate;
	int byte_rate;
	int bit_per_sample;
	int block_align;
	int sample_num;
	int *int4Data;
	short *int2Data;

} wave;

bool isWave(fstream &file)
{
	char char4[5];

	file.seekg(0);
	file.read(char4, 4);
	char4[4] = '\0';

	cout << "File Type: " << char4 << endl;

	if (strcmp(char4, "RIFF"))
		return false;
		
	file.seekg(8);
	file.read(char4, 4);
	char4[4] = '\0';

	cout << "RIFF Type: " << char4 << endl;

	if (strcmp(char4, "WAVE"))
		return false;
		
	return true;

}

void readFmtChunk(fstream &file)
{
	int int4;
	short int2;

	file.read((char*)&int4, 4);
	cout << "Chunk Size: " << int4 << endl;
	
	file.read((char*)&int2, 2);
	cout << "Audio Format: " << int2 << endl;

	file.read((char*)&int2, 2);
	cout << "Channels: " << int2 << endl;
	wave.channels = int2;

	file.read((char*)&int4, 4);
	cout << "Sample rate: " << int4 << endl;
	wave.sample_rate = int4;

	file.read((char*)&int4, 4);
	cout << "Byte rate: " << int4 << endl;
	wave.byte_rate = int4;

	file.read((char*)&int2, 2);
	cout << "Block align: " << int2 << endl;
	wave.block_align = int2;

	file.read((char*)&int2, 2);
	cout << "Bit per sample: " << int2 << endl;
	wave.bit_per_sample = int2;

}

void readDataChunk(fstream &file)
{
	int int4;
	short int2;

	file.read((char*)&int4, 4);
	cout << "Chunk Size: " << int4 << endl;

	int i = 0;
	const int bytePerSample = wave.bit_per_sample / 8;
	cout << "Byte per sample: " << bytePerSample << endl;

	wave.sample_num = int4 / wave.block_align;
	cout << "Numer of samples: " << wave.sample_num << endl;

	switch (bytePerSample)
	{
	case 2:
		wave.int2Data = (short *)malloc(wave.sample_num * sizeof(short));
		file.read((char *)wave.int2Data, wave.sample_num);
		break;
	case 4:
		wave.int4Data = (int *)malloc(wave.sample_num * sizeof(int));
		file.read((char *)wave.int4Data, wave.sample_num);
		break;
	}

	for (int i = 0; i < 10; i++)
	{
		switch (bytePerSample)
		{
		case 2:
			cout << wave.int2Data[i] << endl;
			break;
		case 4:
			cout << wave.int4Data[i] << endl;
			break;
		}
	}

}


void readChunks(fstream &file)
{
	bool gotFmt = false, gotData = false;
	char char4[5];

	file.seekg(12);
	int i = 1;
	while (!gotFmt || !gotData)
	{
		file.read(char4, 4);
		char4[4] = '\0';

		cout << "----------------" << endl;
		cout << "Chunk "<< i++ << " Id: " << char4 << endl;
		if (!strcmp(char4, "fmt ")){
			readFmtChunk(file);
			gotFmt = true;
		}
		else if (!strcmp(char4, "data"))
		{
			readDataChunk(file);
			gotData = true;
		}
		cout << "----------------" << endl;
	}
}

void calculateAverageMagnitude()
{
	int sample;
	for (int i = 0; i < wave.sample_num; i++)
	{
		
	}
}

int main(int argc, char *argv[])
{
	fstream file;
	if (argc == 1){
		cout << "Please input a file (asap.exe file_name)" << endl;
		return 0;
	}
		
	file.open(argv[1], ios::binary | ios::in);

	if (!file.is_open()){
		cout << "File opening error" << endl;
		return 0;
	}

	if (!isWave(file)){
		cout << "This is not a .wav file" << endl;
		return 0;
	}

	readChunks(file);
	calculateAverageMagnitude();

	free(wave.int2Data);
	free(wave.int4Data);
	return 0;
}


