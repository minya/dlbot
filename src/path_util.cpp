#include "path_util.h"

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>


using namespace std;

string expand_path(const string& path) {
    if (path.length() == 0 || path[0] != '~')
      return path;

    const char *pfx = NULL;
    string::size_type pos = path.find_first_of('/');
    if (path.length() == 1 || pos == 1) {
        pfx = getenv("HOME");
        if (!pfx) {
            // Punt. We're trying to expand ~/, but HOME isn't set
            struct passwd *pw = getpwuid(getuid());
            if (pw)
              pfx = pw->pw_dir;
        }
    } else {
        string user(path,1,(pos==string::npos) ? string::npos : pos-1);
        struct passwd *pw = getpwnam(user.c_str());
        if (pw)
          pfx = pw->pw_dir;
    }
    // if we failed to find an expansion, return the path unchanged.
    if (!pfx)
      return path;

    string result(pfx);
    if (pos == string::npos)
      return result;
    if (result.length() == 0 || result[result.length()-1] != '/')
      result += '/';
    result += path.substr(pos+1);
    return result;
}