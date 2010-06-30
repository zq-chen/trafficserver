/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include "inktomi++.h"
#include "I_Layout.h"

static Layout *layout = NULL;

Layout *
Layout::get()
{
  if (layout == NULL) {
    ink_assert("need to call create_default_layout before accessing" "default_layout()");
  }
  return layout;
}

void
Layout::create(const char *prefix)
{
  if (layout == NULL) {
    layout = NEW(new Layout(prefix));
  }
}

static char *
layout_relative(const char *root, const char *file)
{
  char path[PATH_MAX];

  if (ink_filepath_merge(path, PATH_MAX, root, file,
                         INK_FILEPATH_TRUENAME)) {
    int err = errno;
    // Log error
    if (err == EACCES) {
      ink_error("Cannot merge path '%s' above the root '%s'\n", file, root);
    } else if (err == E2BIG) {
      ink_error("Excedding file name length limit of %d characters\n", PATH_MAX);
    }
    else {
      // TODO: Make some pretty errors.
      ink_error("Cannot merge '%s' with '%s' error=%d\n", file, root, err);
    }
    return NULL;
  }
  return xstrdup(path);
}

char *
Layout::relative(const char *file)
{
  return layout_relative(prefix, file);
}

void
Layout::relative(char *buf, size_t bufsz, const char *file)
{
  char path[PATH_MAX];

  if (ink_filepath_merge(path, PATH_MAX, prefix, file,
      INK_FILEPATH_TRUENAME)) {
    int err = errno;
    // Log error
    if (err == EACCES) {
      ink_error("Cannot merge path '%s' above the root '%s'\n", file, prefix);
    } else if (err == E2BIG) {
      ink_error("Excedding file name length limit of %d characters\n", PATH_MAX);
    }
    else {
      // TODO: Make some pretty errors.
      ink_error("Cannot merge '%s' with '%s' error=%d\n", file, prefix, err);
    }
    return;
  }
  size_t path_len = strlen(path) + 1;
  if (path_len > bufsz) {
    ink_error("Provided buffer is too small: %d, required %d\n",
              bufsz, path_len);
  }
  else {
    strcpy(buf, path);
  }
}

char *
Layout::relative_to(const char *dir, const char *file)
{
  return layout_relative(dir, file);
}

void
Layout::relative_to(char *buf, size_t bufsz, const char *dir, const char *file)
{
  char path[PATH_MAX];

  if (ink_filepath_merge(path, PATH_MAX, dir, file,
      INK_FILEPATH_TRUENAME)) {
    int err = errno;
    // Log error
    if (err == EACCES) {
      ink_error("Cannot merge path '%s' above the root '%s'\n", file, dir);
    } else if (err == E2BIG) {
      ink_error("Excedding file name length limit of %d characters\n", PATH_MAX);
    }
    else {
      // TODO: Make some pretty errors.
      ink_error("Cannot merge '%s' with '%s' error=%d\n", file, dir, err);
    }
    return;
  }
  size_t path_len = strlen(path) + 1;
  if (path_len > bufsz) {
    ink_error("Provided buffer is too small: %d, required %d\n",
              bufsz, path_len);
  }
  else {
    strcpy(buf, path);
  }
}

Layout::Layout(const char *_prefix)
{
  if (_prefix) {
    prefix = xstrdup(_prefix);
  }
  else {
    char *env_path;
    char path[PATH_MAX];
    int  len;

    if ((env_path = getenv("TS_ROOT"))) {
      len = strlen(env_path);
      if ((len + 1) > PATH_MAX) {
        ink_error("TS_ROOT environment variable is too big: %d, max %d\n",
                  len, PATH_MAX -1);
        return;
      }
      strcpy(path, env_path);
      while (len > 1 && path[len - 1] == '/') {
        path[len - 1] = '\0';
        --len;
      }
    } else {
        // Use compile time --prefix
        ink_strncpy(path, PREFIX, sizeof(path));
    }

    if (access(path, R_OK) == -1) {
      ink_error("unable to access() TS_ROOT '%s': %d, %s\n",
                path, errno, strerror(errno));
      return;
    }
    prefix = xstrdup(path);
  }
  exec_prefix = layout_relative(prefix, EXEC_PREFIX);
  bindir = layout_relative(prefix, BINDIR);
  sbindir = layout_relative(prefix, SBINDIR);
  sysconfdir = layout_relative(prefix, SYSCONFDIR);
  datadir = layout_relative(prefix, DATADIR);
  includedir = layout_relative(prefix, INCLUDEDIR);
  libdir = layout_relative(prefix, LIBDIR);
  libexecdir = layout_relative(prefix, LIBEXECDIR);
  localstatedir = layout_relative(prefix, LOCALSTATEDIR);
  sharedstatedir = layout_relative(prefix, SHAREDSTATEDIR);
  runtimedir = layout_relative(prefix, RUNTIMEDIR);
  logdir = layout_relative(prefix, LOGDIR);
  mandir = layout_relative(prefix, MANDIR);
  infodir = layout_relative(prefix, INFODIR);
  cachedir = layout_relative(prefix, CACHEDIR);

#ifdef DEBUG
// TODO: Use a propper Debug logging
//
#define PrintSTR(var) \
  fprintf(stdout, "%18s = '%s'\n", "--" #var, (var == NULL? "NULL" : var));

  fprintf(stdout, "Layout configuration\n");
  PrintSTR(prefix);
  PrintSTR(exec_prefix);
  PrintSTR(bindir);
  PrintSTR(sbindir);
  PrintSTR(sysconfdir);
  PrintSTR(datadir);
  PrintSTR(includedir);
  PrintSTR(libdir);
  PrintSTR(libexecdir);
  PrintSTR(localstatedir);
  PrintSTR(sharedstatedir);
  PrintSTR(runtimedir);
  PrintSTR(logdir);
  PrintSTR(mandir);
  PrintSTR(infodir);
  PrintSTR(cachedir);
#endif

}

Layout::~Layout()
{
#define SafeFree(x) \
  if (x) xfree(x);

  SafeFree(prefix);
  SafeFree(exec_prefix);
  SafeFree(bindir);
  SafeFree(sbindir);
  SafeFree(sysconfdir);
  SafeFree(datadir);
  SafeFree(includedir);
  SafeFree(libdir);
  SafeFree(libexecdir);
  SafeFree(localstatedir);
  SafeFree(sharedstatedir);
  SafeFree(runtimedir);
  SafeFree(logdir);
  SafeFree(mandir);
  SafeFree(infodir);
  SafeFree(cachedir);
}

