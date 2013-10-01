/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <edify/expr.h>
#include <minzip/DirUtil.h>
#include <updater/updater.h>

// mkdir(pathname)
Value *MkdirFn(const char *name, State *state, int argc, Expr *argv[]) {
    Value *ret = NULL;
    char *pathname = NULL;

    if (ReadArgs(state, argv, 1, &pathname) < 0) {
        return NULL;
    }

    if (strlen(pathname) == 0) {
        ErrorAbort(state, "pathname argument to %s can't be empty", name);
        goto done;
    }

    if (mkdir(pathname, 0755) < 0) {
        ErrorAbort(state, "%s: cannot create %s", name, pathname);
    }

    ret = StringValue(strdup(""));

done:
    if (pathname)
        free(pathname);

    return ret;
}

// package_extract_file_safe(package_path, destination_path)
//
// this extracts the file with a temporary name first, and
// then renames to overwrite the original file.
Value* PackageExtractFileSafeFn(const char* name, State* state,
                               int argc, Expr* argv[]) {
    if (argc != 2) {
        return ErrorAbort(state, "%s() expects args, got %d",
                          name, argc);
    }

    bool success = false;
    char* zip_path;
    char* dest_path;
    char* dest_path_tmp;

    if (ReadArgs(state, argv, 2, &zip_path, &dest_path) < 0) return NULL;

    if (asprintf(&dest_path_tmp, "%sXXXXXX", dest_path) < 0) return NULL;

    ZipArchive* za = ((UpdaterInfo*)(state->cookie))->package_zip;
    const ZipEntry* entry = mzFindZipEntry(za, zip_path);
    if (entry == NULL) {
        fprintf(stderr, "%s: no %s in package\n", name, zip_path);
        goto done2;
    }

    int tmp_fd = mkstemp(dest_path_tmp);
    if (tmp_fd < 0) {
        fprintf(stderr, "%s: can't open %s for write: %s\n",
                name, dest_path, strerror(errno));
        goto done2;
    }
    success = mzExtractZipEntryToFile(za, entry, tmp_fd);
    close(tmp_fd);

    if (rename(dest_path_tmp, dest_path) < 0) {
        fprintf(stderr, "%s: can't rename %s to %s: %s\n",
                name, dest_path_tmp, dest_path, strerror(errno));
        goto done2;
    }

done2:
    free(zip_path);
    free(dest_path);
    free(dest_path_tmp);
    return StringValue(strdup(success ? "t" : ""));
}

void Register_libcommon_recovery(void)
{
    RegisterFunction("mkdir", MkdirFn);
    RegisterFunction("package_extract_file_safe", PackageExtractFileSafeFn);
}
