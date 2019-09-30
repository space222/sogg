#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <chrono>
#include <thread>
#include <cmath>
#include "types.h"

using std::cout;
using std::string;
using std::vector;

u32 read32(FILE*);
u8  read8(FILE*);
void write32(FILE*, u32);
void write16(FILE*, u16);

template <typename A, std::size_t B>
bool read_array(FILE* fp, std::array<A, B>& arr)
{
	const int len = arr.size()*sizeof(A);
	return len == fread(arr.data(), 1, len, fp);
}

template <typename A, std::size_t B>
bool read_array(FILE* fp, std::array<A, B>& arr, int max)
{
	const int len = max * sizeof(A);
	return len == fread(arr.data(), 1, max, fp);
}

template <typename S>
bool read_struct(FILE* fp, S& truct)
{
	const int len = sizeof(S);
	return len == fread(&truct, 1, len, fp);
}

FILE* outfp;

struct bitstream
{
	explicit bitstream(u32 ser) : serial(ser), last_page(0), stream_length(0), packets(1) {}
	u32 serial;
	u32 last_page;
	u32 stream_length;
	std::vector<std::vector<u8> > packets;
};

std::vector< bitstream > streams;

int main(int argc, char** args)
{
	if( argc < 2 )
	{
		cout << "Need a file to open\n";
		return 1;
	}

	FILE* fp = fopen(args[1], "rb");
	if( !fp )
	{
		cout << "Unable to open <" << args[1] << ">\n";
		return 1;
	}

	std::array<char, 4> magic;
	read_array(fp, magic);
	if( magic[0] != 'O' || magic[1] != 'g' || magic[2] != 'g' || magic[3] != 'S' )
	{
		fclose(fp);
		cout << "Not an Ogg file\n";
		return 1;
	}
	fseek(fp, 0, SEEK_SET);

	cout << "About to unpack Ogg file\n";

	string outfname = args[1];
	auto iter = outfname.rfind(".");
	if( iter != string::npos )
	{
		outfname = outfname.substr(0, iter);
	}
	outfname += ".sgg";

	bool warned = false;

	OggPageHeader head;
	head.header_type = 0;

	while( !feof(fp) && head.header_type != 4 )
	{
		read_struct(fp, head);

		if( head.capt != 0x5367674f )
		{
			cout << "Error: page read fail. " << std::hex << head.capt << " != " << 0x5367674f << '\n' << std::dec;
			fclose(fp);
			return 1;
		}
		
		// find the bitstream for the serial number
		auto index = std::find_if(std::begin(streams), std::end(streams), [&](auto& bs) { return bs.serial == head.serial; });
		
		if( index == std::end(streams) )
		{
			// create new bitstream for serial
			cout << "Info: created new bitstream\n";
			u32 a = head.serial;
			streams.emplace_back(a);
			index = streams.begin() + (streams.size()-1);
		}
		
		bitstream& BS = *index;

		if( !warned && (BS.last_page != head.page_seq /* && head.header_type != 1 */ ) )
		{
			cout << "Warning: page sequence number mismatch\n";
			cout << BS.last_page << " != " << head.page_seq << std::endl;
			warned = true;
		} 
		//cout << BS.last_page << " != " << head.page_seq << std::endl;
		//cout << "header type: " << (int)head.header_type << '\n';
		//if( head.header_type != 1 )
		BS.last_page++;
		
		// read the segment sizes
		std::array<u8, 255> segbuf;
		read_array(fp, segbuf, head.num_segments);

		for(int i = 0; i < head.num_segments; ++i)
		{
			std::vector<u8>& cur_packet = BS.packets.back();
			int sz =(int) cur_packet.size();
			if( segbuf[i] != 0 )
			{
				cur_packet.resize(cur_packet.size() + segbuf[i]);
				int unu = fread(cur_packet.data() + sz, 1, segbuf[i], fp);
			}
			
			if( segbuf[i] != 255 )
			{
				BS.stream_length += cur_packet.size();
				BS.packets.emplace_back();
			}
		}
	}
	fclose(fp);

	cout << "Info: num streams: " << streams.size() << '\n';
	cout << "Info: num packets in stream 0: " << streams[0].packets.size() << '\n';

	// now it's time for a format that makes more sense to a single track Vorbis stream
	outfp = fopen(outfname.c_str(), "wb");

	for(bitstream& BS : streams)
	{
		write32(outfp, BS.serial);
		write32(outfp, BS.stream_length);
		write32(outfp, BS.packets.size());
		for(auto& packet : BS.packets)
		{
			write16(outfp, packet.size());
			int unu = fwrite(packet.data(), 1, packet.size(), outfp);
		}
	}
	
	fclose(outfp);
	return 0;
}

u32 read32(FILE* fp)
{
	u32 a;
	int unu = fread(&a, 1, 4, fp);
	return a;
}

u8 read8(FILE* fp)
{
	u8 a;
	int unu = fread(&a, 1, 1, fp);
	return a;
}

void write32(FILE* fp, u32 val)
{
	int unu = fwrite(&val, 1, 4, fp);
	return;
}

void write16(FILE* fp, u16 val)
{
	int unu = fwrite(&val, 1, 2, fp);
	return;
}


