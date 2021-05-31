#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <boost/program_options.hpp>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>
#include <chrono>
#include <thread>
#include <atomic>

using namespace std;
using namespace boost::program_options;

atomic<unsigned int> kills(0);
unsigned int interval = 0;

int is_file(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

bool is_number(string s) {
    for (unsigned i = 0; i < s.length(); i++)
        if (isdigit(s[i]) == false)
            return false;

    return true;
}

int kill_rust(vector<string>& whitelist) {
    std::vector<std::string> folders;
    DIR* procdir = opendir("/proc");
    struct dirent* entry;
    if (procdir != NULL) {
        while ((entry = readdir(procdir))) {
            if (is_file(entry->d_name) == 0 && is_number(entry->d_name))
                if (getpid() != atoi(entry->d_name))
                    folders.push_back(std::string(entry->d_name));
        }
    }
    closedir(procdir);

    atomic<int> success(0);
    #pragma omp parallel for schedule(simd:runtime) shared(success)
    for (unsigned exe = 0; exe<folders.size(); exe++) {
        ifstream cmdline_file("/proc/" + folders[exe] + "/cmdline");
        string cmdline;
        cmdline_file >> cmdline;
        bool continue_flag = false;
        for (const auto& item : whitelist) {
            if (cmdline.find(item) != string::npos) {
                continue_flag = true;
                break;
            }
        }
        cmdline_file.close();
        if (continue_flag)
            continue;

        if (system(("cat /proc/" + folders[exe] + "/exe 2>/dev/null | grep -a -m 1 -h '/rustc-' 1>/dev/null 2>/dev/null").c_str()) == 0) {
            int result = kill(stoi(folders[exe]), SIGKILL);
            if (result != 0) {
                success = 1;
                perror("kill(2)");
                continue;
            }
            kills++;
        }
    }
    if (kills == 0) {
        success = 1;
    }
    return success;
}

void show_help(options_description& desc, char** argv) {
    cout << "Usage: " << argv[0] << " [options] whitelist...\n";
    cout << desc << flush;
}

int main(int argc, char** argv) {
    signal(SIGINT, [](int signum) {
        cout << "Summary: Killed " << kills << " Rust process(es) in total\n";
        exit(signum);
    });

    options_description desc{"Options"};
    variables_map vm;
    try {
        vector<string> whitelist;
        
        desc.add_options()
            ("help,h", "Help screen")
            ("whitelist", value(&whitelist), "A list of Rust programs to spare")
            ("interval,i", value(&interval), "Kill Rust programs at this interval in milliseconds")
        ;
        positional_options_description p;
        p.add("whitelist", -1);

        store(command_line_parser(argc, argv).
            options(desc).positional(p).run(), vm);
        notify(vm);

        if (vm.count("help")) {
            show_help(desc, argv);
            return 0;
        }
        
        if (interval != 0) {
            for (;;) {
                kill_rust(whitelist);
                this_thread::sleep_for(chrono::milliseconds(interval));
            }
        } else {
            int result = kill_rust(whitelist);
            cout << "Summary: Killed " << kills << " Rust process(es)\n";
            return result;
        }
    } catch(exception& e) {
        if (vm.count("help")) {
            show_help(desc, argv);
            return 0;
        } else {
            cerr << e.what() << endl;
        }
    }
}
