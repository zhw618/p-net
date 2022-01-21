/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

#ifndef VERSION_H
#define VERSION_H

/* #undef OSAL_GIT_REVISION */

#if !defined(OSAL_VERSION_BUILD) && defined(OSAL_VERSION_GIT)
#define OSAL_VERSION_BUILD OSAL_VERSION_GIT
#endif

/* clang-format-off */

#define OSAL_VERSION_MAJOR 0
#define OSAL_VERSION_MINOR 1
#define OSAL_VERSION_PATCH 0

#if defined(OSAL_VERSION_BUILD)
#define OSAL_VERSION \
   "0.1.0+"OSAL_VERSION_BUILD
#else
#define OSAL_VERSION \
   "0.1.0"
#endif

/* clang-format-on */

#endif  /* VERSION_H */
