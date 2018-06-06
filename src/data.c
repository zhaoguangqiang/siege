/**
 * Storage for siege data
 *
 * Copyright (C) 2000-2016 by
 * Jeffrey Fulmer - <jeff@joedog.org>, et al.
 * This file is distributed as part of Siege
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *--
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif/*HAVE_CONFIG_H*/

#include <data.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif/*HAVE_SYS_TIMES_H*/

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif/*HAVE_SYS_TIME_H   */
#endif /*TIME_WITH_SYS_TIME*/


struct DATA_T
{
  float    total;  /*ttime*/
  float    available;
  float    lowest;
  float    highest;
  float    elapsed;
  clock_t  start;   
  clock_t  stop;      
  struct   tms  t_start; 
  struct   tms  t_stop;
  unsigned int  code;
  unsigned int  count;
  unsigned int  okay;
  unsigned int  fail;
  unsigned long long bytes;

  unsigned int  *hits_array;
  unsigned int  hits_array_num;

  float *request_time_array;
  unsigned int  request_time_array_num;
  float percentage_array[9];
  float request_percentage_array[9];
};

DATA
new_data()
{
  DATA this;

  this = calloc(sizeof(*this),1);
  this->total     = 0.0;
  this->available = 0.0;
  this->count     = 0.0;
  this->okay      = 0;
  this->fail      = 0.0;
  this->lowest    =  -1;
  this->highest   = 0.0;
  this->elapsed   = 0.0;
  this->bytes     = 0.0;

  this->hits_array = (unsigned int*)xmalloc(256 * sizeof(unsigned int));
  memset(this->hits_array, 0, 256 * sizeof(unsigned int));
  this->hits_array_num = 0;

  this->request_time_array = NULL;
  this->request_time_array_num = 0;

  this->percentage_array[0] = 0.50;
  this->percentage_array[1] = 0.66;
  this->percentage_array[2] = 0.75;
  this->percentage_array[3] = 0.80;
  this->percentage_array[4] = 0.90;
  this->percentage_array[5] = 0.95;
  this->percentage_array[6] = 0.98;
  this->percentage_array[7] = 0.99;
  this->percentage_array[8] = 1.00;

  return this;
}

DATA
data_destroy(DATA this)
{
  xfree(this->hits_array);
  xfree(this->request_time_array);
  xfree(this);
  return NULL;
} 

void
data_increment_bytes(DATA this, unsigned long bytes)
{
  this->bytes += bytes;
  return;
}

void 
data_increment_count(DATA this, unsigned long count)
{
  this->count += count;
  return;
}

void 
data_increment_total(DATA this, float total)
{
  this->total += total;
  return;
}

void 
data_increment_code(DATA this, int code)
{
  this->code += code;
  return;
}

void 
data_increment_fail(DATA this, int fail)
{
  this->fail += fail;
  return;
}

void
data_increment_okay(DATA this, int okay)
{
  this->okay += okay;
  return;
}

void
data_set_start(DATA this)
{
  this->start = times(&this->t_start);
  return;
}

void
data_set_stop(DATA this)
{
  this->stop = times(&this->t_stop);
  return;
}

void
data_set_highest(DATA this, float highest)
{
  if(this->highest < highest){
    this->highest = highest;
  }
  return;
}

void
data_set_lowest(DATA this, float lowest)
{
  if((this->lowest <= 0)||(this->lowest > lowest)){
    this->lowest = lowest;
  }
  return;
}

void
data_set_hits_array(DATA this, unsigned int *hits_array, unsigned int array_num)
{
  unsigned int i = 0;
  this->hits_array_num = this->hits_array_num > array_num ? this->hits_array_num : array_num;
  
  for (i = 1; i <= array_num; i++)
  {
 //   printf("i:%d--%d,array_num:%d\t", i, hits_array[i], array_num);
    this->hits_array[i] += hits_array[i] - hits_array[i - 1];
  }
  //printf("\n");
}

void
data_set_request_time_array(DATA this, float *request_time_array, unsigned int array_num)
{
  printf("-----------------------array_num:%d, request_num:%d-----------------------------\n", array_num, this->request_time_array_num);
  unsigned int count = this->request_time_array_num;
  this->request_time_array_num += array_num;
  this->request_time_array = (float*)realloc(this->request_time_array, (this->request_time_array_num)*sizeof(float) + 1);
  for (unsigned int i = 0; i < array_num; i++)
    this->request_time_array[count++] = request_time_array[i];
}

int compare_request_time(const void *str1, const void *str2)
{
  float result = *(float*)str1 - *(float*)str2;
  if (result < 0)
    return -1;
  else if (result > 0)
    return 1;
  else
    return 0;
}

void
data_sort_request_time(DATA this)
{
  qsort(this->request_time_array, this->request_time_array_num, sizeof(float), compare_request_time);
  int array_size = sizeof(this->percentage_array)/sizeof(this->percentage_array[0]);
  for (int i = 0; i < array_size; i++){
    int location = this->request_time_array_num * this->percentage_array[i];
    this->request_percentage_array[i] = this->request_time_array[location - 1];
  }
}

float *
data_get_percentage_array(DATA this)
{
  return this->percentage_array;
}

float *
data_get_request_percentage_array(DATA this)
{
  return this->request_percentage_array;
}

unsigned int
data_get_hits_array_num(DATA this)
{
  return this->hits_array_num;
}

unsigned int *
data_get_hits_array(DATA this)
{
  return this->hits_array;
}

unsigned int
data_get_count(DATA this)
{
  return this->count;
}

unsigned int 
data_get_code(DATA this)
{
  return this->code;
}

unsigned int
data_get_fail(DATA this)
{
  return this->fail;
}

unsigned int
data_get_okay(DATA this)
{
  return this->okay;
}

float
data_get_total(DATA this)
{
  return this->total;
}

float
data_get_bytes(DATA this)
{
  return (float)this->bytes;
}

float
data_get_highest(DATA this)
{
  return this->highest;
}

float
data_get_lowest(DATA this)
{
  if(this->code){
    return this->lowest;
  } else {
    return this->code; 
  }
}

float
data_get_megabytes(DATA this)
{
  return (float)this->bytes/(1024.0*1024.0);
}

float
data_get_elapsed(DATA this)
{
  long    tps;
  clock_t time;

  time = this->stop - this->start;
  tps  = sysconf(_SC_CLK_TCK);
  this->elapsed = (float)time/tps;
  return this->elapsed;
}

float
data_get_availability(DATA this)
{
  this->available = (this->count==0)?0:((this->count/(this->count+this->fail))*100);
  return this->available;  
}

float
data_get_response_time(DATA this)
{
  if((this->total==0)||(this->count==0))
    return 0;
  return (this->total / this->count); 
}

float
data_get_transaction_rate(DATA this)
{
  if((this->count==0)||(this->elapsed==0))
    return 0;
  return (this->count / this->elapsed); 
}

float
data_get_throughput(DATA this)
{
  if(this->elapsed==0)
    return 0;
  return this->bytes / (this->elapsed * 1024.0*1024.0);
}

float
data_get_concurrency(DATA this)
{
  if(this->elapsed==0)
    return 0;
  /* total transaction time / elapsed time */
  return (this->total / this->elapsed);
}

