#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
using namespace std;

struct Wave{
	int channels;
	int sample_rate;
	int byte_rate;
	int bit_per_sample;
	int block_align;
	int sample_num;
	int sample_block_num;
	int sample_block_size;
	int sample_block_period;

	int *data;
	double *sample_rms;
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
	int *int4Data;
	short *int2Data;

	file.read((char*)&int4, 4);
	cout << "Chunk Size: " << int4 << endl;

	int i = 0;
	const int bytePerSample = wave.bit_per_sample / 8;
	cout << "Byte per sample: " << bytePerSample << endl;

	wave.sample_num = int4 / wave.block_align;
	cout << "Number of samples: " << wave.sample_num << endl;

	switch (bytePerSample)
	{
	case 2:
		int2Data = (short *)malloc(wave.sample_num * sizeof(short));
		wave.data = (int *)malloc(wave.sample_num * sizeof(int));
		file.read((char *)int2Data, wave.sample_num * bytePerSample);

		for (int i = 0; i < wave.sample_num; i++)
			wave.data[i] = (int)(int2Data[i]);
		break;
	case 4:
		int4Data = (int *)malloc(wave.sample_num * sizeof(int));
		file.read((char *)int4Data, wave.sample_num * bytePerSample);
		wave.data = int4Data;
		break;
	}

	wave.sample_block_period = 10;   //ms
	// Take 1s as a sample block, so there are (sample_rate / byte_rate) sample blocks
	wave.sample_block_size = (int)ceil(wave.sample_rate*wave.sample_block_period/1000.0);
	wave.sample_block_num = (int)ceil( (double)wave.sample_num / wave.sample_block_size);
	cout << "sample block size: " << wave.sample_block_size << endl;
	cout << "sample block num: " << wave.sample_block_num << endl;

}

void readChunks(fstream &file)
{
	bool gotFmt = false, gotData = false;
	char char4[5];
	int int4;

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
		else{
			// Skiping chunks
			file.read((char *)&int4, 4);
			cout << int4 << "to ignore" << endl;
			
			file.ignore(int4);
		}
		cout << "----------------" << endl;
	}
}

void calculateAverageMagnitude()
{
	int r = 0, p = 0, windowPivot = 0;
	double sample_sum = 0.0, average_magnitude = 0.0;
	wave.sample_rms = (double *)malloc(wave.sample_block_num*sizeof(double));

	for (int i = 0; i < wave.sample_block_num; i++) {
		windowPivot = 0;
		sample_sum = 0.0;
		for (r = i*wave.sample_block_size; windowPivot < wave.sample_block_size && r < wave.sample_num; r++, windowPivot++){
			sample_sum += (wave.data[r])*(wave.data[r]);
		}
		sample_sum /= windowPivot;
		sample_sum = sqrt(sample_sum);
		/*
		cout << wave.sample_block_period*i << "ms " 
			<< "from " << i*wave.sample_block_size
			<< " to " << r << ": " << sample_sum << endl;
		*/
		(wave.sample_rms)[p++] = sample_sum;
	}

	cout << "sample rms: ";
	for (int i = 0; i < wave.sample_block_num; i++)
		cout << wave.sample_rms[i] << ", ";
	cout << endl;

	sample_sum = 0;
	cout << "sample rms rms: ";
	for (int i = 0; i < wave.sample_block_num; i++){
		sample_sum += wave.sample_rms[i] * wave.sample_rms[i];
	}
	sample_sum /= wave.sample_block_num;
	sample_sum = sqrt(sample_sum);
	cout << sample_sum;
	cout << endl;
}

void searchForSlienceBlock()
{
	const double threshold = 100;
	int consecutiveBlock = 0;
	int totalSlienceBlock = 0;
	bool sliencing;
	bool *slienceBlock = (bool *)malloc(wave.sample_block_num*sizeof(bool));
	for (int i = 0; i < wave.sample_block_num; i++) {
		slienceBlock[i] = wave.sample_rms[i] < threshold;
	}

	for (int i = 0; i < wave.sample_block_num; i++) {
		sliencing = false;
		for (; i < wave.sample_block_num; i++) {
			if (slienceBlock[i] == true) {
				if (!sliencing) {
					totalSlienceBlock+=consecutiveBlock;
					consecutiveBlock = 1;
					sliencing = true;
					cout << wave.sample_block_period*i<<"ms through ";
				}
				consecutiveBlock++;
			}
			else{
				if(sliencing) { 
					cout << wave.sample_block_period*i << "ms" 
					<< " --- " << consecutiveBlock << " blocks" << endl;
				}
				break;
			}
		}
	}

	if (sliencing==true){
		totalSlienceBlock += consecutiveBlock;
		cout << "end" << " --- "<< consecutiveBlock << " blocks" << endl;
	}
	cout << "Total silence block: " << totalSlienceBlock << endl;
	cout << "Total silence time: "  << totalSlienceBlock*wave.sample_block_period/1000.0 << "s" << endl;
	cout << "Total silence percentage: " << (float)totalSlienceBlock/wave.sample_block_num*100 << "%" << endl;
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
		cout << "This is not a Wave file" << endl;
		return 0;
	}

	readChunks(file);
	calculateAverageMagnitude();
	searchForSlienceBlock();

	free(wave.data);
	return 0;
}


