#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Find the path to the executable itself
void get_executable_dir(char *dir_path, size_t max_len) {
#if defined(__linux__) || defined(__CYGWIN__)
    ssize_t count = readlink("/proc/self/exe", dir_path, max_len);
    if (count != -1) {
        dir_path[count] = '\0';
        char *last_slash = strrchr(dir_path, '/');
        if (last_slash) *last_slash = '\0';
    } else {
        dir_path[0] = '\0';
    }
#elif defined(_WIN32)
    GetModuleFileName(NULL, dir_path, max_len);
    char *last_slash = strrchr(dir_path, '\\');
    if (!last_slash) last_slash = strrchr(dir_path, '/');
    if (last_slash) *last_slash = '\0';
#else
    dir_path[0] = '\0';
#endif
}

int main(int argc, char *argv[]) {
    // 1. Find the --protocol argument to determine which binary to route to
    char protocol[32] = {0};
    
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--protocol") == 0 || strcmp(argv[i], "-p") == 0) {
            strncpy(protocol, argv[i+1], sizeof(protocol) - 1);
            break;
        }
    }

    if (protocol[0] == '\0') {
        // If the user is just asking for list, help, or version, route to NGAP by default
        if (argc > 1 && (strcmp(argv[1], "list") == 0 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "version") == 0)) {
            strncpy(protocol, "ngap", sizeof(protocol) - 1);
        } else {
            fprintf(stderr, "Error: Missing --protocol argument. Please specify a protocol (e.g., NGAP, X2AP).\n");
            return 1;
        }
    }

    // Convert protocol to lowercase
    for(int i = 0; protocol[i]; i++) {
        protocol[i] = tolower(protocol[i]);
    }

    // Map RRC sub-protocols to 'rrc' binary
    const char* binary_suffix = protocol;
    if (strcmp(protocol, "ul_ccch") == 0 || strcmp(protocol, "dl_ccch") == 0 ||
        strcmp(protocol, "ul_dcch") == 0 || strcmp(protocol, "dl_dcch") == 0 ||
        strcmp(protocol, "bcch_bch") == 0 || strcmp(protocol, "bcch_dl_sch") == 0 ||
        strcmp(protocol, "pcch") == 0) {
        binary_suffix = "rrc";
    }

    // 2. Find our directory so we can run the sibling binary
    char dir_path[PATH_MAX];
    get_executable_dir(dir_path, sizeof(dir_path));

    char target_binary[PATH_MAX];
    if (dir_path[0] != '\0') {
        snprintf(target_binary, sizeof(target_binary), "%s/asncodec_%s", dir_path, binary_suffix);
    } else {
        snprintf(target_binary, sizeof(target_binary), "./asncodec_%s", binary_suffix);
    }

    // 3. Execute the target binary with all original arguments
    argv[0] = target_binary; // Conventionally pass the true path as argv[0]
    execv(target_binary, argv);
    
    // If execv returns, it failed
    fprintf(stderr, "Error: Failed to route to '%s'. Is the protocol supported and built?\n", target_binary);
    perror("execv");
    return 1;
}
