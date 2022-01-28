#include "FileSys.h"
#include <cassert>

namespace MF
{
//FCB::FCB(byte* p) : FCB(p, 0) {
//}

FCB::FCB(byte* p, index_t ind)
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

FCB::FCB(byte*      p, const index_t ind,
         const char fn[], const byte isdir, const time_t updtime, const index_t father, uint32_t size)
	: FCB(p, ind) {
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

//FCB::FCB(index_t n_index, const char fn[], const byte dir, const time_t up, const index_t father, uint32_t size)
//	: FCB(FileSys::p_curMFileImg.get() + n_index, fn, dir, up, father, size) {
//}
//
//
//FCB::FCB(index_t n_index, const string& fn, const byte dir, const time_t up, const index_t father, uint32_t size)
//	: FCB(FileSys::p_curMFileImg.get() + n_index, fn.data(), dir, up, father, size) {
//}

//FCB::FCB(FCB& f): ptr(f.ptr), index(f.index),
//                  p_file_name(f.p_file_name), p_isdir(f.p_isdir), p_updtime(f.p_updtime),
//                  p_father(f.p_father), p_size(f.p_size), p_nodeindexs(f.p_nodeindexs) {
//
//}

//index_t FCB::find_1_avail_entrance() {
//	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i)
//		if (p_nodeindexs[i] == 0)
//			return i;
//	//todo
//}

const char* FCB::file_name() const { return reinterpret_cast<const char*>(p_file_name); }

void FCB::set_file_name(const char fn[]) {
	memset(p_file_name, 0, 115);
	memcpy(p_file_name, fn, strlen(fn) + 1);
}

char FCB::isdir() const { return *reinterpret_cast<const char*>(p_isdir); }


void FCB::set_isdir(char isdir) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_isdir = isdir;
#else
		memcpy(p_isdir, &isdir, 1);
#endif
}

time_t FCB::updtime() const { return *p_updtime; }

void FCB::set_updtime(time_t updtime) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_updtime = updtime;
#else
		memcpy(p_updtime, &updtime, 8);
#endif
}

index_t FCB::father() const { return *p_father; }

void FCB::set_father(index_t father) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_father = father;
#else
		memcpy(p_father, &father, 2);
#endif
}

uint32_t FCB::size() const { return *p_size; }

void FCB::set_size(uint32_t size) {
#if MF_ASSIGN_INDEAD_OF_MEMCPY==true
	*p_size = size;
#else
		memcpy(p_size, &size, 4);
#endif
}

index_t* FCB::nodeindexs() const { return p_nodeindexs; }

//shared_ptr<byte> FileSys::p_curMFileImg{};

//�õ�����·����
string FilePath::get() {
	string ret;
	for (auto& s : data) {
		ret += s;
		ret += "/";
	}

	return ret;
}

//�õ����ڲ���ļ�����
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
	//FCB_in_cwd = vector<FCB>(FILESYS_BLK_INDEX_NUM, FCB());
	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) { FCB_in_cwd[i] = FCB(); }
	cwd += "";

	//index_left_avail = 0;
	//index_right_aval = 0;
	//memset(this->MFileImg, 0, sizeof(this->MFileImg));
	//memset(this->bitmap, 0, sizeof(this->bitmap));
	_set_blk_used(0);
	cwdFCB = FCB(MFileImg, 0, "", true, 0, 0, 0);
}

FileSys::~FileSys() {
	//delete MFileImg;
}

bool FileSys::save(string img_name) {
	if (img_name.size() > 7 &&
	    img_name.substr(img_name.size() - 7, 7) == ".mfsimg");
	//���û����չ���ͼ���
	else { img_name += ".mfsimg"; }
	std::ofstream bfo(img_name, std::fstream::out | std::fstream::binary);
	if (bfo.is_open()) {
		//����MFileImg��bitmap��Ӳ��
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
	//�ļ��������չ��
	if (img_name.size() > 7 &&
	    img_name.substr(img_name.size() - 7, 7) == ".mfsimg");
	//���û����չ���ͼ���
	else { img_name += ".mfsimg"; }
	auto          ptr = new FileSys(img_name);
	std::ifstream bfi(img_name, std::fstream::in | std::fstream::binary);
	if (bfi.is_open()) {
		bfi.read(reinterpret_cast<char*>(ptr->MFileImg), FILESYS_CAPACITY_BYTES);
		bfi.read(reinterpret_cast<char*>(ptr->bitmap), sizeof(ptr->bitmap));
		if (bfi.good()) {
			//ˢ��cwd����ػ���
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
	//if (blks_aval <= 0 || blks_aval >= FILESYS_BLKS_NUM) {
	//	std::cerr << "Error:\t no space on this filesystem" << std::endl;
	//	return -3;
	//}
	//auto    p   = cwdFCB.nodeindexs();
	//int     cnt = 0;
	//index_t i   = 0;
	//for (; i < FILESYS_BLK_INDEX_NUM; ++i) {
	//	if (p[i] == 0) break;
	//	++cnt;
	//	if (strcmp(fn.data(), FCB_in_cwd[i].file_name())) {
	//		std::cerr << "Error:\t this block is NOT FCB" << std::endl;
	//		return -1;
	//	}
	//}
	//if (cnt >= FILESYS_BLK_INDEX_NUM) {
	//	std::cerr << "Error:\t this dir is full" << std::endl;
	//	return -2;
	//}
	//if (index_left_avail == 0) {
	//	index_left_avail = _find_1_avail_blk_front();
	//	if (index_left_avail == 0) {
	//		std::cerr << "Error:\t no space on this filesystem" << std::endl;
	//		return -3;
	//	}
	//}
	//_set_blk_used(index_left_avail);
	//p[i] = index_left_avail;
}

int FileSys::rmdir(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//�����տ�
		if (f.index == 0)continue;
		//�ҵ���ӦFCB���������ļ���
		if (f.file_name() == fn && f.isdir() == 1) {
			//����ļ������滹���ļ�����ɾ��
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				auto z = f.nodeindexs()[i];
				if (z != 0) {
					std::cerr << "Error:\t cannot delete non-empty dir" << std::endl;
					return false;
				}
			}
			//����ϼ�Ŀ¼FCB�ڵ����
			auto pt = cwdFCB.nodeindexs();
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				if (pt[i] == f.index) {
					pt[i] = 0;
					break;
				}
			}
			//��������ڶ�Ӧ�ļ���FCB
			_clear_blk(f.index);
			f.index = 0;
			//�����Ӧ�ļ���FCB����
			f = FCB();
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
		//�����ļ����������ֺ��渽��һ��"/"
		if (f.isdir())s += "/";
		auto ttt = f.updtime();
		ret.emplace_back(vector<string>{s, std::to_string(f.size()), time2string(ttt)});
	}
	return ret;
}

int FileSys::cd(const string& ss) {
	//���·����һ�ν�һ���ļ��У�
	if (ss[0] != '/') {
		//cd ..������һ��
		if (ss.size() == 2 && ss[0] == '.' && ss[1] == '.' ||
		    ss.size() == 3 && ss[0] == '.' && ss[1] == '.' && ss[2] == '/') {
			if (cwdFCB.index == 0) {
				std::cerr << "Error:\t already in root" << std::endl;
				return false;
			}
			////����cwd
			//cwdFCB = FCB(MFileImg, cwdFCB.father());;
			////����cwdָ���FCB
			//auto p = cwdFCB.nodeindexs();
			//for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
			//	if (p[i] == 0)
			//		FCB_in_cwd[i] = FCB();
			//	else
			//		FCB_in_cwd[i] = FCB(MFileImg, p[i]);
			//}
			_refresh_cwd_FCB_cache(cwdFCB.father());
			//����cwd·����¼
			--cwd;
			return true;
		}
		//�ж��ļ����Ϸ�
		if (is_legal_fn(ss))
		//������cd
			return _cd(ss);
		else {
			std::cerr << "Error:\t illegal path" << std::endl;
			return false;
		}
	}
	//todo
	//����·����"/aa/ss/cc/"��"/aa/xx/ss"��
	else {
		//����'cd /'
		if (ss.size() == 1 && ss == "/") {
			cwd = FilePath();
			cwd += "";
			_refresh_cwd_FCB_cache(0);
			return true;
		}
		//�������·���Ϸ������ν�����һ�㣬��û�����ŵ�"/"
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

bool FileSys::open(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//�����տ�
		if (f.index == 0)continue;
		//�ҵ���ӦFCB�����Ҳ����ļ���
		if (f.file_name() == fn && f.isdir() == 0) {
			//�ļ���δ����?
			for (auto& ff : open_file_list) {
				if (ff.index == f.index) {
					std::cerr << "Error:\t file is already open" << std::endl;
					return false;
				}
			}
			open_file_list.push_back(f);
			return true;
		}
	}
	std::cerr << "Error:\t no such file in cwd" << std::endl;
	return false;
}

int FileSys::close(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//�����տ�
		if (f.index == 0)continue;
		//�ҵ���ӦFCB�����Ҳ����ļ���
		if (f.file_name() == fn && f.isdir() == 0) {
			//�ļ��ѱ���?
			//for (auto iter = open_file_list.begin(); iter != open_file_list.end();) {
			//	//С�ĵ�����ʧЧ��
			//	if (iter->index == f.index) { iter = open_file_list.erase(iter); }
			//	++iter;
			//}
			for (int i = 0; i < open_file_list.size(); ++i) {
				if (f.index == open_file_list[i].index) {
					open_file_list.erase(open_file_list.begin() + i);
					return true;
				}
			}
		}
	}
	std::cerr << "Error:\t no such file in cwd" << std::endl;
	return false;
}

vector<byte> FileSys::read(const string fn) {
	for (const auto& s : FCB_in_cwd) {
		//��������Ŀ
		if (s.index == 0)continue;
		//�ҵ���ӦFCB�����Ҳ������ļ���
		if (fn == s.file_name() && s.isdir() == 0) {
			//�Ѿ��򿪣�
			if (!_is_open(fn)) {
				std::cerr << "Error:\t file not open" << std::endl;
				return vector<byte>();
			}
			int             size = s.size();
			vector<byte>    ret(size, 0);
			vector<index_t> blks;
			int             cnt = 0;
			//һ��һ�����vector<byte>��
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

			if (cnt < size)std::cerr << "Warning:\t file real size is NOT equal to FCB" << std::endl;
			return ret;
		}
	}
	std::cerr << "Error:\t no such path existed" << std::endl;
	return {};
}

int FileSys::write(const string& fn, const string& str) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//�����տ�
		if (f.index == 0)continue;
		//�ҵ���ӦFCB�����Ҳ������ļ���
		if (f.file_name() == fn && f.isdir() == 0) {
			//�Ѿ��򿪣�
			if (!_is_open(fn)) {
				std::cerr << "Error:\t file not open" << std::endl;
				return false;
			}
			size_t size = str.size() + 1;
			//�������ļ���������
			if (size > FILESYS_FILE_MAXLEN) {
				std::cerr << "Error:\t exceed max file length" << std::endl;
				return false;
			}
			//�ҵ���д��Ŀ�
			vector<index_t> blks;
			while (blks.size() < std::ceil((double)size / (double)FILESYS_BLK_SIZE_BYTES)) {
				auto t = _find_1_avail_blk_front();
				//�Ҳ������򷵻���Щ��
				if (t == 0) {
					std::cerr << "Error:\t no enough space for this file" << std::endl;
					for (auto i : blks)_set_blk_unused(i);
					return false;
				}
				blks.push_back(t);
				_set_blk_used(t);
			}
			//FCB��¼�ļ���С
			f.set_size(size);
			//���ΰ��ļ�����д���
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
	}
	std::cerr << "Error:\t no such file existed" << std::endl;
	return false;
}

int FileSys::del(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	for (auto& f : FCB_in_cwd) {
		//�����տ�
		if (f.index == 0)continue;
		//�ҵ���ӦFCB�����Ҳ������ļ���
		if (f.file_name() == fn && f.isdir() == 0) {
			//�ļ�δopen?
			for (auto& ff : open_file_list) {
				if (f.index == ff.index) {
					std::cerr << "Error: cannot delete open file" << std::endl;
					return false;
				}
			}
			//����������д洢�ļ����ݵ�block
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				auto z = f.nodeindexs()[i];
				if (z != 0) {
					_clear_blk(z);
					_set_blk_unused(z);
				}
			}
			//����ϼ�Ŀ¼FCB�ڵ����
			auto pt = cwdFCB.nodeindexs();
			for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
				if (pt[i] == f.index) {
					pt[i] = 0;
					break;
				}
			}
			//��������ڶ�Ӧ�ļ���FCB
			_clear_blk(f.index);
			//�����Ӧ�ļ���FCB����
			f = FCB();

			return true;
		}
	}
	std::cerr << "Error:\t file not found" << std::endl;
	return false;
}

string FileSys::get_cwd_str() { return cwd.get(); }

bool FileSys::_cd(const string& fn) {
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal path" << std::endl;
		return false;
	}
	for (const auto& s : FCB_in_cwd) {
		//��������Ŀ
		if (s.index == 0)continue;
		//�ļ�����ͬ��Ϊ�ļ���
		if (fn == s.file_name() && s.isdir()) {
			////����cwd
			//cwdFCB = s;
			////����cwdָ���FCB
			//auto p = cwdFCB.nodeindexs();
			//for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
			//	if (p[i] == 0)
			//		FCB_in_cwd[i] = FCB();
			//	else
			//		FCB_in_cwd[i] = FCB(MFileImg, p[i]);
			//}
			_refresh_cwd_FCB_cache(s.index);
			//����cwd·����¼
			cwd += fn;
			return true;
		}
	}
	std::cerr << "Error:\t no such path existed" << std::endl;
	return false;
}

bool FileSys::_newfile(const string& fn, bool dir) {
	//�ж��ļ����Ƿ�Ϸ�
	if (!is_legal_fn(fn)) {
		std::cerr << "Error:\t illegal file name" << std::endl;
		return false;
	}
	int  avalentry = 0, i = 0;
	bool found     = false;
	//����cwd��FCB�Ŀ�����Ŀ
	for (const auto& f : FCB_in_cwd) {
		if (f.index == 0) {
			avalentry = (found) ? avalentry : i; //FCB���ҵ�һ������Ŀ
			found     = true;
			continue;
		}
		if (fn == f.file_name()) {
			std::cerr << "Error:\t a file with same name exists in cwd" << std::endl;
			return false;
		}
		++i;
	}
	//���ҿ��д��̿������ļ���FCB
	auto avalblk = _find_1_avail_blk_front();
	if (!found) {
		std::cerr << "Error:\t this dir is full" << std::endl;
		return false;
	}
	if (avalblk == 0) {
		std::cerr << "Error:\t no space on this filesystem" << std::endl;
		return false;
	}
	//�������ļ�FCB��д��飬����cwdFCB��������cwdFCB��������Ŀ
	_set_blk_used(avalblk);
	FCB_in_cwd[avalentry]          = FCB(MFileImg, avalblk, fn.data(), dir, local_time_now(), cwdFCB.index, 0);
	cwdFCB.nodeindexs()[avalentry] = avalblk;
	return true;
}

bool FileSys::_refresh_cwd_FCB_cache(const index_t index) {
	//����cwd
	cwdFCB = FCB(MFileImg, index);;
	//����cwdָ���FCB
	auto p = cwdFCB.nodeindexs();
	for (int i = 0; i < FILESYS_BLK_INDEX_NUM; ++i) {
		if (p[i] == 0) {
			FCB_in_cwd[i]       = FCB();
			FCB_in_cwd[i].index = 0;
		}
		else
			FCB_in_cwd[i] = FCB(MFileImg, p[i]);
	}
	return true;
}

bool FileSys::_is_open(const string& fn) {
	for (auto& f : open_file_list) { if (f.file_name() == fn) { return true; } }
	return false;
}

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

//����bitmap�жϿ��Ƿ�ռ��
inline bool FileSys::_is_blk_used(const index_t index) const {
	return (bitmap[index / 8] & MASK[index % 8]) == MASK[index % 8];
}

//����bitmap�п鱻ռ��
inline void FileSys::_set_blk_used(const index_t index) { bitmap[index / 8] |= MASK[index % 8]; }

//����bitmap�п����
inline void FileSys::_set_blk_unused(const index_t index) { bitmap[index / 8] &= ~MASK[index % 8]; }

//��ǰ����ʼɨ��bitmap���ҵ�һ�����õĿ飬�Ҳ����ͷ���0
//todo �����Ż�
inline index_t FileSys::_find_1_avail_blk_front() {
	for (index_t i = 1; i < FILESYS_BLKS_NUM; ++i)
		if (!_is_blk_used(i))
			return i;
	return 0;
}

//�Ӻ���ǰ��ʼɨ��bitmap���ҵ�һ�����õĿ飬�Ҳ����ͷ���0
//todo �����Ż�
inline index_t FileSys::_find_next_1_blk_back() {
	for (index_t i = FILESYS_BLKS_NUM - 1; i > 0; --i)
		if (!_is_blk_used(i))
			return i;
	return 0;
}
}