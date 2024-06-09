#ifndef __ZEMB_TRACER_LESS_H__
#define __ZEMB_TRACER_LESS_H__
/******************************************************************************
* Copyright(C) 2021-2025 ZBeetle. All Rights Reserved.
* @file   : //文件名 
* @brief  : //简介 
* @version: //版本号
* @author : //作者
* @date   : //完成日期
* @history:
* > author@20210106 修复xxx问题. 
* 
******************************************************************************/

/* 此文件用于消除打印,在实时系统中,为了满足系统实时性,我们有时有必要关闭串口打印.
 * 这种情况下，我们只需要在源码中包含本文件(在include "Tracer.h"之后),即可立即消
 * 除打印.示例代码:
 * #include "Tracer.h"
 * #include "TracerLess.h" //调试时我们可以屏蔽这句,那样会正常打印
 * ...
 * TRACE_DBG("..."); //当我们包含TracerLess.h时,此时将不打印
 * ...
 */

#ifdef TRACE_DBG
#undef TRACE_DBG
#endif
#define TRACE_DBG(fmt, arg...)

#ifdef TRACE_DBG_CLASS
#undef TRACE_DBG_CLASS
#endif
#define TRACE_DBG_CLASS(fmt, arg...)

#ifdef TRACE_ASSERT
#undef TRACE_ASSERT
#endif
#define TRACE_ASSERT(condition)

#endif
