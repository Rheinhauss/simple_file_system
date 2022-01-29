#include <iostream>

#include "FileSys.h"
#include "utils.h"

using namespace std;
using namespace MF;

int main(int argc, const char** argv) {
	string c, a, b;
	//shared_ptr<MF::FileSys> ffs;
	FileSys* ffs = nullptr;
	//ģʽ��0=�����½��ļ�ϵͳ����(new, sfs)��1=�ļ�ϵͳ�ڲ���(����exit)
	int mode = 0;
	//cout << MF::time2string(MF::local_time_now()) << endl;
	while (true) {
		cin >> c;
		if (mode == 0) {
			if (c == "new") {
				cin >> a;
				//ffs = make_shared<FileSys>(FileSys::newfsys());
				ffs  = FileSys::newfsys(a);
				mode = 1;

				//todo
			}
			else if (c == "sfs") {
				cin >> a;
				ffs = FileSys::load(a);
				if (ffs != nullptr)
					mode = 1;
				//todo load
			}
		}
		else if (mode == 1) {
			if (c == "exit") {
				ffs->save();
				mode = 0;
			}
			else if (c == "mkdir") {
				cin >> a;
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				ffs->mkdir(a);
			}
			else if (c == "rmdir") {
				cin >> a;
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				ffs->rmdir(a);
			}
			else if (c == "ls") {
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				auto arr_str_ls = ffs->ls();
				for (auto& str_file : arr_str_ls) {
					for (auto& ele : str_file) { cout << ele << "\t"; }
					cout << endl;
				}
				//cout << endl;
			}
			else if (c == "cd") {
				cin >> a;
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				ffs->cd(a);
			}
			else if (c == "create") {
				cin >> a;
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				ffs->create(a);
			}
			else if (c == "open") {
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				cin >> a;
				cout << "fd: " << ffs->open(a) << endl;
			}
			else if (c == "close") {
				string fdstr;
				cin >> fdstr;
				int fd = -1;
				if (MF::is_number(fdstr)) {
					fd = stoi(fdstr);
					if (ffs == nullptr) {
						cerr << "Error:\t no filesystem is open" << endl;
						continue;
					}
					ffs->close(fd);
				}
				else { cerr << "Error:\t fd must be a integer" << endl; }
			}
			else if (c == "read") {
				string fdstr;
				cin >> fdstr;
				int fd = -1;
				if (MF::is_number(fdstr)) {
					fd = stoi(fdstr);
					if (ffs == nullptr) {
						cerr << "Error:\t no filesystem is open" << endl;
						continue;
					}
					auto ans = ffs->read(fd);
					for (auto& ch : ans) { cout << static_cast<char>(ch); }
					cout << endl;
				}
				else { cerr << "Error:\t fd must be a integer" << endl; }
			}
			else if (c == "write") {
				string fdstr;
				cin >> fdstr;
				int fd = -1;
				if (MF::is_number(fdstr)) {
					fd = stoi(fdstr);
					if (ffs == nullptr) {
						cerr << "Error:\t no filesystem is open" << endl;
						continue;
					}
					if (ffs->is_fd_open(fd)) {
						cout << "Info:\t use CTRL+D (*nix) or CTRL+Z - ENTER (Windows) to end input" << endl;
						string stmp;
						b = "";
						while (std::getline(std::cin, stmp)) { b += stmp; }
						ffs->write(fd, b);
						// ����cin����EOF״̬
						cin.clear();
					}
				}
				else { cerr << "Error:\t fd must be a integer" << endl; }
			}
			else if (c == "delete") {
				cin >> a;
				if (ffs == nullptr) {
					cerr << "Error:\t no filesystem is open" << endl;
					continue;
				}
				ffs->del(a);
			}
			else;
		}
		if (mode == 1 && ffs != nullptr) {
			//ģ��shellˢ��һ������
			cout << endl << ffs->get_cwd_str() << " $ ";
		}
	}
}
