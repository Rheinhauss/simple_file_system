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
constexpr size_t FILESYS_FD_MAX          = 128; //���ͬʱ��128���ļ�

using byte = unsigned char; // ���ڱ�ʾ������С��ʾ��Ԫ��byte
using index_t = uint16_t;   // ���ڱ�ʾ�������Ϊ������Ϊ4096������2�ֽڳ����޷������� 0-65535 �㹻ʹ�á�

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


//FCBp ��һ������FCB�Ľṹ�壬���ǲ���ʵ�ʴ洢FCB������
struct FCBp
{
public:
	//Ĭ�Ϲ��캯��
	explicit FCBp() = default;
	explicit FCBp(byte* p);
	//���ݿ������������ڴ澵��ͷָ�룬��������Ӧ�Ŀ齨��FCB�ṹ��
	explicit FCBp(byte* p, index_t ind);
	//���ݿ������������ڴ澵��ͷָ�롢��Ҫ�ڴ����Ͻ�����FCB����Ϣ����������Ӧ�Ŀ���д��FCB
	explicit FCBp(byte* p, index_t ind, const char fn[], byte isdir, time_t updtime, index_t father, uint32_t size);


	FCBp(const FCBp& f) = default;

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


	const char* file_name() const;              //���ڴ��еĴ��̾����ȡ�ļ���
	void        set_file_name(const char fn[]); //�������ļ������ڴ��еĴ��̾���
	char        isdir() const;                  //���ڴ��еĴ��̾����ȡ�Ƿ�Ϊ�ļ���
	void        set_isdir(char isdir);          //�����µ��Ƿ��ļ��е��ڴ��еĴ��̾���
	time_t      updtime() const;                //���ڴ��еĴ��̾����ȡ����޸�ʱ��
	void        set_updtime(time_t updtime);    //�����µ��޸�ʱ�䵽�ڴ��еĴ��̾���
	index_t     father() const;                 //���ڴ��еĴ��̾����ȡ�ϼ�Ŀ¼
	void        set_father(index_t father);     //�����µ��ϼ�Ŀ¼���ڴ��еĴ��̾���
	uint32_t    size() const;                   //���ڴ��еĴ��̾����ȡ�ļ���С
	void        set_size(uint32_t size);        //�������ļ���С���ڴ��еĴ��̾���
	index_t*    nodeindexs() const;             //���ڴ��еĴ��̾����ȡFCB�ں�������

private:
};

struct FilePath
{
	vector<string> data;   //ÿһ���ļ��е�string��������,��Ŀ¼Ϊ���ַ���
	string         get();  //�õ�����"/dir1/dir2/dir3/"���ַ���
	string         last(); //�õ����ڲ���ļ�����

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
	int                    open(const string& fn);
	int                    close(int fd);
	vector<byte>           read(int fd);
	int                    write(int fd, const string& str); //����д
	int                    del(const string& fn);

	int    is_fd_open(int fd);
	string get_cwd_str(); //�����ⲿ��ȡcwd

protected:
	bool _cd(const string& fn);                 //���·��cd
	bool _newfile(const string& fn, bool dir);  //��Ϊmkdir��create��������Ϊ�˴��븴��д���������
	bool _refresh_cwd_FCB_cache(index_t index); //����cwd�������ļ���FCB����
	//bool _is_open(const string& fn);            //�ж��ļ��Ƿ����Ѵ��ļ��б���

	inline bool    _is_blk_used(index_t index) const; //����bitmap�жϿ��Ƿ�ռ��
	inline void    _set_blk_used(index_t index);      //����bitmap�п鱻ռ��
	inline void    _set_blk_unused(index_t index);    //����bitmap�п����
	inline void    _clear_blk(index_t index);         //���block����
	inline index_t _find_1_avail_blk_front();         //��ǰ����ʼɨ��bitmap���ҵ�һ�����õĿ飬�Ҳ����ͷ���0
	inline index_t _find_next_1_blk_back();           //�Ӻ���ǰ��ʼɨ��bitmap���ҵ�һ�����õĿ飬�Ҳ����ͷ���0
	inline int     _find_1_avail_fd();                //Ѱ��һ�����õ�fd�š�����0��Ϊû�п��õ���

	inline static bool is_legal_fn(const string& fn);       //�ж��ļ����Ƿ�Ϸ�
	inline static bool is_legal_fn_slash(const string& fn); //�ж��ļ����Ƿ�Ϸ����Ϸ��ַ���һ��'/'
	inline static bool is_legal_path(const string& p);      //�ж�·���Ƿ�Ϸ�


	//FCBp& get_FCB_from_index(const index_t index) const {}

	const unsigned char MASK[8] = {
		0b10000000u, 0b01000000u,
		0b00100000u, 0b00010000u,
		0b00001000u, 0b00000100u,
		0b00000010u, 0b00000001u
	}; //bitmap�������

	unsigned char bitmap[FILESYS_BLKS_NUM / 8]{};     //���bitmap������
	byte          MFileImg[FILESYS_CAPACITY_BYTES]{}; //�ڴ��еĴ��̾���
	FilePath      cwd;                                //��ǰ�����ļ���·��
	FCBp          cwdFCB;                             //��ǰ�����ļ��е�FCB
	FCBp          FCB_in_cwd[FILESYS_BLK_INDEX_NUM];  //��ǰ�����ļ������ļ���FCB����
	vector<FCBp>  open_file_list;                     //�Ѵ��ļ���FCB, 0��Ĭ���Ǹ�Ŀ¼
	string        fs_name;                            //��ǰ�ļ�ϵͳ�����֣����ڱ���Ͷ�ȡ�ļ�ϵͳ����
};
}
