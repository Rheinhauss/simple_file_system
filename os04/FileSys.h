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
constexpr size_t FILESYS_FD_MAX          = 128; //最多同时打开128个文件

using byte = unsigned char; // 用于表示磁盘最小表示单元，byte
using index_t = uint16_t;   // 用于表示索引项。因为块总数为4096，所以2字节长的无符号整数 0-65535 足够使用。

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


//FCBp 是一个类似FCB的结构体，但是并不实际存储FCB的内容
struct FCBp
{
public:
	//默认构造函数
	explicit FCBp() = default;
	explicit FCBp(byte* p);
	//根据块索引、磁盘内存镜像头指针，从索引对应的块建立FCB结构体
	explicit FCBp(byte* p, index_t ind);
	//根据块索引、磁盘内存镜像头指针、需要在磁盘上建立的FCB的信息，在索引对应的块上写入FCB
	explicit FCBp(byte* p, index_t ind, const char fn[], byte isdir, time_t updtime, index_t father, uint32_t size);


	FCBp(const FCBp& f) = default;

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


	const char* file_name() const;              //从内存中的磁盘镜像读取文件名
	void        set_file_name(const char fn[]); //设置新文件名到内存中的磁盘镜像
	char        isdir() const;                  //从内存中的磁盘镜像读取是否为文件夹
	void        set_isdir(char isdir);          //设置新的是否文件夹到内存中的磁盘镜像
	time_t      updtime() const;                //从内存中的磁盘镜像读取最近修改时间
	void        set_updtime(time_t updtime);    //设置新的修改时间到内存中的磁盘镜像
	index_t     father() const;                 //从内存中的磁盘镜像读取上级目录
	void        set_father(index_t father);     //设置新的上级目录到内存中的磁盘镜像
	uint32_t    size() const;                   //从内存中的磁盘镜像读取文件大小
	void        set_size(uint32_t size);        //设置新文件大小到内存中的磁盘镜像
	index_t*    nodeindexs() const;             //从内存中的磁盘镜像读取FCB内含的索引

private:
};

struct FilePath
{
	vector<string> data;   //每一级文件夹的string单独保存,根目录为空字符串
	string         get();  //得到形如"/dir1/dir2/dir3/"的字符串
	string         last(); //得到最内层的文件夹名

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
	int                    open(const string& fn);
	int                    close(int fd);
	vector<byte>           read(int fd);
	int                    write(int fd, const string& str); //覆盖写
	int                    del(const string& fn);

	int    is_fd_open(int fd);
	string get_cwd_str(); //方便外部读取cwd

protected:
	bool _cd(const string& fn);                 //相对路径cd
	bool _newfile(const string& fn, bool dir);  //因为mkdir和create很像，所以为了代码复用写了这个函数
	bool _refresh_cwd_FCB_cache(index_t index); //更新cwd中所含文件的FCB缓存
	//bool _is_open(const string& fn);            //判断文件是否在已打开文件列表里

	inline bool    _is_blk_used(index_t index) const; //利用bitmap判断块是否被占用
	inline void    _set_blk_used(index_t index);      //设置bitmap中块被占用
	inline void    _set_blk_unused(index_t index);    //设置bitmap中块可用
	inline void    _clear_blk(index_t index);         //清除block内容
	inline index_t _find_1_avail_blk_front();         //从前往后开始扫描bitmap，找到一个可用的块，找不到就返回0
	inline index_t _find_next_1_blk_back();           //从后往前开始扫描bitmap，找到一个可用的块，找不到就返回0
	inline int     _find_1_avail_fd();                //寻找一个可用的fd号。返回0即为没有可用的了

	inline static bool is_legal_fn(const string& fn);       //判断文件名是否合法
	inline static bool is_legal_fn_slash(const string& fn); //判断文件名是否合法，合法字符多一个'/'
	inline static bool is_legal_path(const string& p);      //判断路径是否合法


	//FCBp& get_FCB_from_index(const index_t index) const {}

	const unsigned char MASK[8] = {
		0b10000000u, 0b01000000u,
		0b00100000u, 0b00010000u,
		0b00001000u, 0b00000100u,
		0b00000010u, 0b00000001u
	}; //bitmap相关掩码

	unsigned char bitmap[FILESYS_BLKS_NUM / 8]{};     //存放bitmap的数组
	byte          MFileImg[FILESYS_CAPACITY_BYTES]{}; //内存中的磁盘镜像
	FilePath      cwd;                                //当前所在文件夹路径
	FCBp          cwdFCB;                             //当前所在文件夹的FCB
	FCBp          FCB_in_cwd[FILESYS_BLK_INDEX_NUM];  //当前所在文件夹内文件的FCB缓存
	vector<FCBp>  open_file_list;                     //已打开文件的FCB, 0号默认是根目录
	string        fs_name;                            //当前文件系统的名字，用于保存和读取文件系统镜像
};
}
