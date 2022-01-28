#pragma once
#include <string>
//#include <ctime>
#include <chrono>
//#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

#include "utils.h"

#define MF_ASSIGN_INDEAD_OF_MEMCPY true

namespace MF
{
using std::string;
//using std::time;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::make_shared;
using std::make_unique;
using std::vector;


constexpr size_t FILESYS_BLK_INDEX_NUM  = 448;
constexpr size_t FILESYS_BLK_SIZE_BYTES = 1 << 10; // 1 KB
constexpr size_t FILESYS_BLKS_NUM       = 1 << 12; // 4096 blocks
constexpr size_t FILESYS_CAPACITY_BYTES = FILESYS_BLK_SIZE_BYTES
                                          * FILESYS_BLKS_NUM; // 4 MB, ����bitmap��512B
constexpr size_t FILESYS_FILENAME_MAXLEN = 113;               // including '\0'
constexpr size_t FILESYS_FILE_MAXLEN     = FILESYS_BLK_INDEX_NUM * FILESYS_BLK_SIZE_BYTES;

typedef unsigned char byte;
typedef uint16_t      index_t;

//ȷ��������ȷ������Ϊ64λ����
static_assert(
	sizeof(uint32_t) == 4 &&
	sizeof(uint16_t) == 2 &&
	sizeof(uint64_t) == 8 &&
	sizeof(byte) == 1 &&
	sizeof(size_t) == 8 &&
	sizeof(void*) == 8 &&
	sizeof(time_t) == 8 &&
	sizeof(bool) == 1 &&
	sizeof(index_t) == 2,
	"type length ERROR"
);

//struct Block
//{
//public:
//	byte s[1024]{0};
//};

//struct FCB {
//public:
//	FCB();
//	FCB(const char fn[], bool dir, time_t up, uint32_t father);
//	char p_file_name[115]{ };	// 115B, including '\0'
//	char p_isdir = 0;			// 1B��Ϊ0����ͨ�ļ���Ϊ1��Ŀ¼
//	time_t p_updtime = 0;		// 8B, int64
//	uint32_t p_father = 0;   // 4B��Ϊ�ļ��ĸ�Ŀ¼����Ŀ¼�ĸ�Ŀ¼Ϊ0
//	uint32_t p_nodeindexs[FILESYS_BLK_INDEX_NUM]{ }; // 768B = 4*224B
//	// 0 ���� ��
//	// p_isdir == false ʱ��
//	//		p_nodeindexs �洢�ļ���������һ���������ļ����244KB
//	// p_isdir == true  ʱ��
//	//		p_nodeindexs �洢FCB���ڿ��������ļ��������Ϊ224
//	uint32_t find_1_avail_entrance();
//};a

struct FCB
{
private:
public:
	explicit FCB() = default;
	explicit FCB(byte* p);
	explicit FCB(byte* p, index_t ind);
	explicit FCB(byte* p, index_t ind, const char fn[], byte isdir, time_t updtime, index_t father, uint32_t size);
	//explicit FCB(index_t n_index, const char fn[], byte isdir, time_t updtime, index_t father, uint32_t size);
	//explicit FCB(index_t n_index, const string& fn, byte isdir, time_t updtime, index_t father, uint32_t size);

	FCB(const FCB& f) = default;

	byte*   ptr   = nullptr; // ����ʼ��ַ
	index_t index = 0;       // �ڼ���

	// ���ݹ� 1024 B
	char*     p_file_name  = nullptr; // 113 = FILESYS_FILENAME_MAXLEN*1B
	char*     p_isdir      = nullptr; // 1*1B��Ϊ0����ͨ�ļ���Ϊ1��Ŀ¼
	time_t*   p_updtime    = nullptr; // 1*8B, ����ʱ��
	index_t*  p_father     = nullptr; // 1*2B��Ϊ�ļ��ĸ�Ŀ¼����Ŀ¼�ĸ�Ŀ¼Ϊ0
	uint32_t* p_size       = nullptr; // 1*4B���ļ���С���ֽڣ�
	index_t*  p_nodeindexs = nullptr; // 896B = FILESYS_BLK_INDEX_NUM*2B
	// 0 ���� ��
	// p_isdir == 0 ʱ��
	//		p_nodeindexs �洢�ļ���������һ���������ļ����244KB
	// p_isdir == 1  ʱ��
	//		p_nodeindexs �洢FCB���ڿ��������ļ��������Ϊ224

	//index_t find_1_avail_entrance();


	const char* file_name() const;
	void        set_file_name(const char fn[]);
	char        isdir() const;
	void        set_isdir(char isdir);
	time_t      updtime() const;
	void        set_updtime(time_t updtime);
	index_t     father() const;
	void        set_father(index_t father);
	uint32_t    size() const;
	void        set_size(uint32_t size);
	index_t*    nodeindexs() const;

private:
};

struct FilePath
{
	vector<string> data;
	string         get();
	string         last();

	FilePath& operator+=(const string& fn); // ����һ��
	FilePath& operator--();                 // �˳�һ��
};

class FileSys
{
public:
	FileSys(const string& fsn);
	~FileSys();

	bool                   save() { return save(fs_name); }
	bool                   save(string img_name);
	static FileSys*        load(string img_name);
	static FileSys*        newfsys(const string& fsn);
	int                    mkdir(const string& fn);
	int                    rmdir(const string& fn);
	vector<vector<string>> ls();
	int                    cd(const string& ss);
	int                    create(const string& fn);
	bool                   open(const string& fn);
	int                    close(const string& fn);
	vector<byte>           read(const string fn);
	int                    write(const string& fn, const string& str); //����д
	int                    del(const string& fn);
	string                 get_cwd_str();

protected:
	FilePath cwd;
	FCB      cwdFCB;
	FCB      FCB_in_cwd[FILESYS_BLK_INDEX_NUM];
	bool     _cd(const string& fn);
	bool     _newfile(const string& fn, bool dir);
	bool     _refresh_cwd_FCB_cache(const index_t index);
	bool     _is_open(const string& fn);

	inline bool    _is_blk_used(const index_t index) const; // index start at 0
	inline void    _set_blk_used(const index_t index);
	inline void    _set_blk_unused(const index_t index);
	inline void    _clear_blk(const index_t index); //���block����
	inline index_t _find_1_avail_blk_front();
	inline index_t _find_next_1_blk_back();

	inline static bool is_legal_fn(const string& fn);
	inline static bool is_legal_fn_slash(const string& fn);
	inline static bool is_legal_path(const string& p);


	//FCB& get_FCB_from_index(const index_t index) const {}

	const unsigned char MASK[8] = {
		0b10000000u, 0b01000000u,
		0b00100000u, 0b00010000u,
		0b00001000u, 0b00000100u,
		0b00000010u, 0b00000001u
	}; //bitmap�������

	unsigned char bitmap[FILESYS_BLKS_NUM / 8]{};     //���bitmap������
	byte          MFileImg[FILESYS_CAPACITY_BYTES]{}; // �ڴ��еĴ��̾���

	vector<FCB> open_file_list;
	string      fs_name;
};
}
