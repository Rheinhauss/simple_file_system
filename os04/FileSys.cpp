#include "FileSys.h"
#include <cassert>

namespace MF
{
//FCBp::FCBp(byte* p) : FCBp(p, 0) {
//}

FCBp::FCBp(byte* p, index_t ind)
	: ptr(p + ind * FILESYS_BLK_SIZE_BYTES),
	  index(ind),
	  p_file_name(reinterpret_cast<decltype(p_file_name)>(ptr)),
	  p_isdir(reinterpret_cast<decltype(p_isdir)>(ptr + FILESYS_FILENAME_MAXLEN)),
	  p_updtime(reinterpret_cast<decltype(p_updtime)>(ptr + (FILESYS_FILENAME_MAXLEN + 1))),
	  p_father(reinterpret_cast<decltype(p_father)>(ptr + (FILESYS_FILENAME_MAXLEN + 1 + 8))),
	  p_size(reinterpret_cast<decltype(p_size)>(ptr + (FILESYS_FILENAME_MAXLEN + 1 + 8 + 2))),
	  p_nodeindexs(
			  reinterpret_cast<decltype(p_nodeindexs)>(
				  ptr + (FILESYS_FILENAME_MAXLEN + 1 + 8 + 2 + 4))) {

}

FCBp::FCBp(byte*      p, const index_t ind,
           const char fn[], const byte isdir, const time_t updtime, const index_t father, uint32_t size)
	: FCBp(p, ind) {
	assert(strlen(fn) <= FILESYS_FILENAME_MAXLEN - 1);
	memset(p_file_name, 0, FILESYS_FILENAME_MAXLEN);
	memcpy(p_file_name, fn, strlen(fn) + 1);
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_isdir   = isdir;
	*p_updtime = updtime;
	*p_father  = father;
	*p_size    = size;
#else
	memcpy(p_isdir, &isdir, decltype(*p_isdir));//1
	memcpy(p_updtime, &updtime, decltype(*p_updtime));//8
	memcpy(p_father, &father, decltype(*p_father));//2
#endif
	memset(p_nodeindexs, 0, FILESYS_BLK_INDEX_NUM * sizeof(decltype(*p_nodeindexs))); //896=448*2
}

//FCBp::FCBp(index_t n_index, const char fn[], const byte dir, const time_t up, const index_t father, uint32_t size)
//	: FCBp(FileSys::p_curMFileImg.get() + n_index, fn, dir, up, father, size) {
//}
//
//
//FCBp::FCBp(index_t n_index, const string& fn, const byte dir, const time_t up, const index_t father, uint32_t size)
//	: FCBp(FileSys::p_curMFileImg.get() + n_index, fn.data(), dir, up, father, size) {
//}

//FCBp::FCBp(FCBp& f): ptr(f.ptr), index(f.index),
//                  p_file_name(f.p_file_name), p_isdir(f.p_isdir), p_updtime(f.p_updtime),
//                  p_father(f.p_father), p_size(f.p_size), p_nodeindexs(f.p_nodeindexs) {
//
//}

//index_t FCBp::find_1_avail_entrance() {
//	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i)
//		if (p_nodeindexs[i] == 0)
//			return i;
//	//todo
//}

const char* FCBp::file_name() const { return reinterpret_cast<const char*>(p_file_name); }

void FCBp::set_file_name(const char fn[]) {
	memset(p_file_name, 0, 115);
	memcpy(p_file_name, fn, strlen(fn) + 1);
}

char FCBp::isdir() const { return *reinterpret_cast<const char*>(p_isdir); }


void FCBp::set_isdir(char isdir) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_isdir = isdir;
#else
		memcpy(p_isdir, &isdir, 1);
#endif
}

time_t FCBp::updtime() const { return *p_updtime; }

void FCBp::set_updtime(time_t updtime) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_updtime = updtime;
#else
		memcpy(p_updtime, &updtime, 8);
#endif
}

index_t FCBp::father() const { return *p_father; }

void FCBp::set_father(index_t father) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_father = father;
#else
		memcpy(p_father, &father, 2);
#endif
}

uint32_t FCBp::size() const { return *p_size; }

void FCBp::set_size(uint32_t size) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_size = size;
#else
		memcpy(p_size, &size, 4);
#endif
}

index_t* FCBp::nodeindexs() const { return p_nodeindexs; }

//shared_ptr<byte> FileSys::p_curMFileImg{};

//得到绝对路径名
string FilePath::get() {
	string ret;
	for (auto& s : data) {
		ret += s;
		ret += "/";
	}

	return ret;
}

//得到最内层的文件夹名
string FilePath::last() { return data[data.size() - 1]; }

FilePath& FilePath::operator+=(const string& fn) {
	data.push_back(fn);
	return *this;
}

FilePath& FilePath::operator--() {
	if (data.empty()) {
		std::cerr << "Warning:\t path is already root" << std::endl;
		return *this;
	}
	else {
		data.pop_back();
		return *this;
	}
}

FileSys::FileSys(const string& fsn) {
	fs_name = fsn;
	memset(MFileImg, 0, sizeof(MFileImg));
	//FCB_in_cwd = vector<FCBp>(FILESYS_BLK_INDEX_NUM, FCBp());
	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) { FCB_in_cwd[i] = FCBp(); }
	cwd += "";

	//index_left_avail = 0;
	//index_right_aval = 0;
	//memset(this->MFileImg, 0, sizeof(this->MFileImg));
	//memset(this->bitmap, 0, sizeof(this->bitmap));
	_set_blk_used(0);
	cwdFCB = FCBp(MFileImg, 0, "", true, 0, 0, 0);

	open_file_list    = vector<FCBp>(FILESYS_FD_MAX, FCBp());
	open_file_list[0] = cwdFCB;
}

FileSys::~FileSys() {
	//delete MFileImg;
}

bool FileSys::save(string img_name) {
	if (img_name.size() > 7 &&
	    img_name.substr(img_name.size() - 7, 7) == ".mfsimg");
	//如果没有拓展名就加上
	else { img_name += ".mfsimg"; }
	std::ofstream bfo(img_name, std::fstream::out | std::fstream::binary);
	if (bfo.is_open()) {
		//保存MFileImg和bitmap到硬盘
		bfo.write(reinterpret_cast<const char*>(MFileImg), FILESYS_CAPACITY_BYTES);
		bfo.write(reinterpret_cast<const char*>(bitmap), sizeof(bitmap));
		if (bfo.good()) {
			std::cerr << "Info:\t file system \"" + fs_name + "\""
					+ " saved as \"" + img_name + "\"" << std::endl;
			return true;
		}
	}
	std::cerr << "Error:\t failed to save this file system" << std::endl;
	return false;
}

FileSys* FileSys::load(string img_name) {
	//文件如果有拓展名
	if (img_name.size() > 7 &&
	    img_name.substr(img_name.size() - 7, 7) == ".mfsimg");
	//如果没有拓展名就加上
	else { img_name += ".mfsimg"; }
	auto          ptr = new FileSys(img_name);
	std::ifstream bfi(img_name, std::fstream::in | std::fstream::binary);
	if (bfi.is_open()) {
		bfi.read(reinterpret_cast<char*>(ptr->MFileImg), FILESYS_CAPACITY_BYTES);
		bfi.read(reinterpret_cast<char*>(ptr->bitmap), sizeof(ptr->bitmap));
		if (bfi.good()) {
			//刷新cwd与相关缓存
			ptr->cd("/");
			return ptr;
		}
	}
	std::cerr << "Error:\t failed to load this file system" << std::endl;
	return nullptr;
}

FileSys* FileSys::newfsys(const string& fsn) { return (new FileSys(fsn)); }

int FileSys::mkdir(const string& fn) {
	return _newfile(fn, true);
}

int FileSys::rmdir(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//跳过空块
		if (f.index == 0)continue;
		//找到对应FCB！而且是文件夹
		if (f.file_name() == fn && f.isdir() == 1) {
			//如果文件夹里面还有文件则不能删除
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				auto z = f.nodeindexs()[i];
				if (z != 0) {
					std::cerr << "Error:\t cannot delete non-empty dir" << std::endl;
					return false;
				}
			}
			//清除上级目录FCB内的入口
			auto pt = cwdFCB.nodeindexs();
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				if (pt[i] == f.index) {
					pt[i] = 0;
					break;
				}
			}
			//清除磁盘内对应文件的FCB
			_clear_blk(f.index);
			f.index = 0;
			//清除对应文件的FCB缓存
			f = FCBp();
			return true;
		}
	}
	std::cerr << "Error:\t no such file" << std::endl;;
	return false;
}

vector<vector<string>> FileSys::ls() {
	vector<vector<string>> ret;
	for (const auto& f : FCB_in_cwd) {
		if (f.index == 0)continue;
		auto s = string(f.file_name());
		//若是文件夹则在名字后面附带一个"/"
		if (f.isdir())s += "/";
		auto ttt = f.updtime();
		ret.emplace_back(vector<string>{s, std::to_string(f.size()), time2string(ttt)});
	}
	return ret;
}

int FileSys::cd(const string& ss) {
	//相对路径（一次进一个文件夹）
	if (ss[0] != '/') {
		//cd ..返回上一级
		if (ss.size() == 2 && ss[0] == '.' && ss[1] == '.' ||
		    ss.size() == 3 && ss[0] == '.' && ss[1] == '.' && ss[2] == '/') {
			if (cwdFCB.index == 0) {
				std::cerr << "Error:\t already in root" << std::endl;
				return false;
			}
			//更新cwd和cwd内含文件的FCB缓存
			_refresh_cwd_FCB_cache(cwdFCB.father());
			//更新cwd路径记录
			--cwd;
			return true;
		}
		//判断文件名合法
		if (is_legal_fn(ss))
		//真正的cd
			return _cd(ss);
		else {
			std::cerr << "Error:\t illegal path" << std::endl;
			return false;
		}
	}
	//todo
	//绝对路径（"/aa/ss/cc/"或"/aa/xx/ss"）
	else {
		//特判'cd /'
		if (ss.size() == 1 && ss == "/") {
			cwd = FilePath();
			cwd += "";
			_refresh_cwd_FCB_cache(0);
			return true;
		}
		//初步检查路径合法后依次进入下一层，即没有连着的"/"
		auto vss = split(ss, '/');
		for (int i = 0; i < ss.size(); ++i) {
			if (vss[i].size() == 0 && !(i == 0 || i == vss.size() - 1)) {
				std::cerr << "Error:\t illegal path" << std::endl;
				return false;
			}
		}
		int z = vss.back().size() == 0 ? 1 : 0;
		for (int i = 1; i < vss.size() - z; ++i) { if (!_cd(vss[i])) { return false; } }
	}
}

int FileSys::create(const string& fn) { return _newfile(fn, false); }

int FileSys::open(const string& fn) {
	int fd = 0;
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//跳过空块
		if (f.index == 0)continue;
		//找到对应FCB！而且不是文件夹
		if (f.file_name() == fn && f.isdir() == 0) {
			//文件还未被打开,若打开则返回那个fd
			for (int i = 1; i < FILESYS_FD_MAX; ++i) {
				auto& ff = open_file_list[i];
				if (ff.index == 0)continue;
				if (ff.index == f.index) {
					std::cerr << "Error:\t file is already open" << std::endl;
					return i;
				}
			}
			for (int i = 1; i < FILESYS_FD_MAX; ++i) {
				auto& ff = open_file_list[i];
				if (ff.index == 0) {
					ff = f;
					return i;
				}
			}
		}
	}
	std::cerr << "Error:\t no such file in cwd" << std::endl;
	return 0;
}

int FileSys::close(int fd) {
	if (!(fd > 0 && fd < FILESYS_FD_MAX && open_file_list[fd].index != 0)) {
		std::cerr << "Error:\t fd error" << std::endl;
		return false;
	}
	open_file_list[fd] = FCBp();
	open_file_list[fd].index = 0;
	return true;
}

vector<byte> FileSys::read(int fd) {
	//fd在范围内且已经打开
	if (!(fd > 0 && fd < FILESYS_FD_MAX && open_file_list[fd].index != 0)) {
		std::cerr << "Error:\t fd error" << std::endl;
		return {};
	}
	auto&           s    = open_file_list[fd];
	int             size = s.size();
	vector<byte>    ret(size, 0);
	vector<index_t> blks;
	int             cnt = 0;
	//一块一块读到vector<byte>里
	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; i++) {
		index_t ind = s.nodeindexs()[i];
		if (ind != 0) {
			for (int i = 0; i < FILESYS_BLK_SIZE_BYTES; ++i) {
				if (cnt < size) {
					ret[cnt] = MFileImg[ind * FILESYS_BLK_SIZE_BYTES + i];
					++cnt;
				}
				else return ret;
			}
		}
	}

	if (cnt < size)std::cerr << "Warning:\t file real size is NOT equal to FCBp" << std::endl;
	return ret;
}

int FileSys::write(int fd, const string& str) {
	//fd在范围内且已经打开
	if (!(fd > 0 && fd < FILESYS_FD_MAX && open_file_list[fd].index != 0)) {
		std::cerr << "Error:\t fd error" << std::endl;
		return {};
	}
	auto& f = open_file_list[fd];
	size_t size = str.size() + 1;
	//超出单文件长度限制
	if (size > FILESYS_FILE_MAXLEN) {
		std::cerr << "Error:\t exceed max file length" << std::endl;
		return false;
	}
	//找到供写入的块
	vector<index_t> blks;
	while (blks.size() < std::ceil((double)size / (double)FILESYS_BLK_SIZE_BYTES)) {
		auto t = _find_1_avail_blk_front();
		//找不够，则返还这些块
		if (t == 0) {
			std::cerr << "Error:\t no enough space for this file" << std::endl;
			for (auto i : blks)_set_blk_unused(i);
			return false;
		}
		blks.push_back(t);
		_set_blk_used(t);
	}
	//FCB记录文件大小
	f.set_size(size);
	//依次把文件内容写入块
	size_t now = 0, end = 0;
	for (int i = 0; i < blks.size(); ++i) {
		end    = std::min(now + FILESYS_BLK_SIZE_BYTES, size);
		auto z = blks[i] * FILESYS_BLK_SIZE_BYTES;
		//memcpy(MFileImg + z, str.data() + now, end - now);
		for (int ii = now; ii < end; ++ii) { MFileImg[z + ii] = str[now + ii]; }
		now               = end;
		f.nodeindexs()[i] = blks[i];
	}
	return true;
}

int FileSys::del(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//跳过空块
		if (f.index == 0)continue;
		//找到对应FCB！而且不能是文件夹
		if (f.file_name() == fn && f.isdir() == 0) {
			//文件未open?
			for (auto& ff : open_file_list) {
				if (f.index == ff.index) {
					std::cerr << "Error: cannot delete open file" << std::endl;
					return false;
				}
			}
			//依次清除所有存储文件内容的block
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				auto z = f.nodeindexs()[i];
				if (z != 0) {
					_clear_blk(z);
					_set_blk_unused(z);
				}
			}
			//清除上级目录FCB内的入口
			auto pt = cwdFCB.nodeindexs();
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				if (pt[i] == f.index) {
					pt[i] = 0;
					break;
				}
			}
			//清除磁盘内对应文件的FCB
			_clear_blk(f.index);
			//清除对应文件的FCB缓存
			f = FCBp();

			return true;
		}
	}
	std::cerr << "Error:\t file not found" << std::endl;
	return false;
}

int FileSys::is_fd_open(int fd) {
	if (!(fd > 0 && fd < FILESYS_FD_MAX)) {
		std::cerr << "Error:\t fd error" << std::endl;
		return false;
	}
	if (open_file_list[fd].index != 0)return true;
	return false;
}

string FileSys::get_cwd_str() { return cwd.get(); }

bool FileSys::_cd(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal path" << std::endl;
		return false;
	}
	for (const auto& s : FCB_in_cwd) {
		//跳过空条目
		if (s.index == 0)continue;
		//文件名相同且为文件夹
		if (fn == s.file_name() && s.isdir()) {
			//更新cwd
			//更新cwd指向的FCB
			_refresh_cwd_FCB_cache(s.index);
			//更新cwd路径记录
			cwd += fn;
			return true;
		}
	}
	std::cerr << "Error:\t no such path existed" << std::endl;
	return false;
}

bool FileSys::_newfile(const string& fn, bool dir) {
	//判断文件名是否合法
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	int  avalentry = 0, i = 0;
	bool found     = false;
	//先找cwd的FCB的空闲条目
	for (const auto& f : FCB_in_cwd) {
		if (f.index == 0) {
			avalentry = (found) ? avalentry : i; //FCB里找第一个空条目
			found     = true;
			continue;
		}
		if (fn == f.file_name()) {
			std::cerr << "Error:\t a file with same name exists in cwd" << std::endl;
			return false;
		}
		++i;
	}
	//再找空闲磁盘块存放新文件的FCB
	auto avalblk = _find_1_avail_blk_front();
	if (!found) {
		std::cerr << "Error:\t this dir is full" << std::endl;
		return false;
	}
	if (avalblk == 0) {
		std::cerr << "Error:\t no space on this filesystem" << std::endl;
		return false;
	}
	//创建新文件FCB，写入块，更新cwdFCB，并更新cwdFCB包含的条目
	_set_blk_used(avalblk);
	FCB_in_cwd[avalentry]          = FCBp(MFileImg, avalblk, fn.data(), dir, local_time_now(), cwdFCB.index, 0);
	cwdFCB.nodeindexs()[avalentry] = avalblk;
	return true;
}

bool FileSys::_refresh_cwd_FCB_cache(const index_t index) {
	//更新cwd
	cwdFCB = FCBp(MFileImg, index);;
	//更新cwd指向的FCB
	auto p = cwdFCB.nodeindexs();
	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
		if (p[i] == 0) {
			FCB_in_cwd[i]       = FCBp();
			FCB_in_cwd[i].index = 0;
		}
		else
			FCB_in_cwd[i] = FCBp(MFileImg, p[i]);
	}
	return true;
}

//bool FileSys::_is_open(const string& fn) {
//	for (auto& f : open_file_list) { if (f.file_name() == fn) { return true; } }
//	return false;
//}

void FileSys::_clear_blk(const index_t index) {
	memset(MFileImg + index * FILESYS_BLK_SIZE_BYTES, 0, FILESYS_BLK_SIZE_BYTES);
}

bool FileSys::is_legal_fn(const string& fn) {
	if (fn.empty())
		return false;
	for (char c : fn) {
		if (
			!(
				c == 46 ||              // '.'
				(48 <= c && c <= 57) || // 0-9
				(65 <= c && c <= 90) || // A-Z
				(97 <= c && c <= 122)   // a-z
			)
		)
			return false;
	}
	return true;
}

bool FileSys::is_legal_fn_slash(const string& fn) {
	if (fn.empty())
		return false;
	for (char c : fn) {
		if (
			!(
				c == 46 ||              // '.'
				c == 47 ||              // '/'
				(48 <= c && c <= 57) || // 0-9
				(65 <= c && c <= 90) || // A-Z
				(97 <= c && c <= 122)   // a-z
			)
		)
			return false;
	}
	return true;
}

bool FileSys::is_legal_path(const string& p) {
	if (p[0] == '/') { return is_legal_fn_slash(p); }
	else return is_legal_fn(p);
}


inline bool FileSys::_is_blk_used(const index_t index) const {
	return (bitmap[index / 8] & MASK[index % 8]) == MASK[index % 8];
}


inline void FileSys::_set_blk_used(const index_t index) { bitmap[index / 8] |= MASK[index % 8]; }


inline void FileSys::_set_blk_unused(const index_t index) { bitmap[index / 8] &= ~MASK[index % 8]; }


//todo 性能优化
inline index_t FileSys::_find_1_avail_blk_front() {
	for (index_t i = 1; i < FILESYS_BLKS_NUM; ++i)
		if (!_is_blk_used(i))
			return i;
	return 0;
}


//todo 性能优化
inline index_t FileSys::_find_next_1_blk_back() {
	for (index_t i = FILESYS_BLKS_NUM - 1; i > 0; --i)
		if (!_is_blk_used(i))
			return i;
	return 0;
}

inline int FileSys::_find_1_avail_fd() {
	for (int i = 0; i < FILESYS_FD_MAX; ++i) {
		if (open_file_list[i].index == 0)continue;
		return i;
	}
	return 0;
}
}
