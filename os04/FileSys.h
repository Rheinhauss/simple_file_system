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
                                          * FILESYS_BLKS_NUM; // 4 MB, 不含bitmap的512B
constexpr size_t FILESYS_FILENAME_MAXLEN = 113;               // including '\0'
constexpr size_t FILESYS_FILE_MAXLEN     = FILESYS_BLK_INDEX_NUM * FILESYS_BLK_SIZE_BYTES;

typedef unsigned char byte;
typedef uint16_t      index_t;

//确保长度正确，必须为64位程序
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
//	char p_isdir = 0;			// 1B，为0是普通文件，为1是目录
//	time_t p_updtime = 0;		// 8B, int64
//	uint32_t p_father = 0;   // 4B，为文件的父目录，根目录的父目录为0
//	uint32_t p_nodeindexs[FILESYS_BLK_INDEX_NUM]{ }; // 768B = 4*224B
//	// 0 代表 空
//	// p_isdir == false 时，
//	//		p_nodeindexs 存储文件块索引，一级索引，文件最大244KB
//	// p_isdir == true  时，
//	//		p_nodeindexs 存储FCB所在块索引，文件个数最多为224
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

	byte*   ptr   = nullptr; // 块起始地址
	index_t index = 0;       // 第几块

	// 数据共 1024 B
	char*     p_file_name  = nullptr; // 113 = FILESYS_FILENAME_MAXLEN*1B
	char*     p_isdir      = nullptr; // 1*1B，为0是普通文件，为1是目录
	time_t*   p_updtime    = nullptr; // 1*8B, 更新时间
	index_t*  p_father     = nullptr; // 1*2B，为文件的父目录，根目录的父目录为0
	uint32_t* p_size       = nullptr; // 1*4B，文件大小（字节）
	index_t*  p_nodeindexs = nullptr; // 896B = FILESYS_BLK_INDEX_NUM*2B
	// 0 代表 空
	// p_isdir == 0 时，
	//		p_nodeindexs 存储文件块索引，一级索引，文件最大244KB
	// p_isdir == 1  时，
	//		p_nodeindexs 存储FCB所在块索引，文件个数最多为224

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

	FilePath& operator+=(const string& fn); // 进入一层
	FilePath& operator--();                 // 退出一层
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
	int                    write(const string& fn, const string& str); //覆盖写
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
	inline void    _clear_blk(const index_t index); //清除block内容
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
	}; //bitmap相关掩码

	unsigned char bitmap[FILESYS_BLKS_NUM / 8]{};     //存放bitmap的数组
	byte          MFileImg[FILESYS_CAPACITY_BYTES]{}; // 内存中的磁盘镜像

	vector<FCB> open_file_list;
	string      fs_name;
};
}
