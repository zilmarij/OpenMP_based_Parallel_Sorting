#include<omp.h>
#include <iostream>
#include<math.h>
#include<cstring>
#include "sort.h"


const int size = 20; // omp_get_num_procs();

int part(pSort::dataType* rbuf, int low, int high);
void QUICk(pSort::dataType* rbuf, int l, int h);

pSort::dataType* slice(pSort::dataType* rbuf, int start, int end);
void merge(pSort::dataType* result, pSort::dataType* left, pSort::dataType* right, int leftLen, int rightLen);
void MERGe(pSort::dataType* rbuf, int len);

void place(pSort::dataType* rbuf, int place_val, int c);
void RADIx(pSort::dataType* rbuf, int c);
void pSort::init()
{

}


void pSort::sort(pSort::dataType* rbuf, int l, pSort::SortType type)
{
	omp_set_dynamic(0);
	omp_set_num_threads(size);
	if (type == 1 || type == 0)
	{
#pragma omp parallel //firstprivate(rank)
		{

#pragma omp single //critical(sec1)  //single  //task  //task untied
			{
				QUICk(rbuf, 0, l - 1);
			}
		}
	}
	else if (type == 2)
	{
#pragma omp parallel 
		{
#pragma omp single 
			{
				MERGe(rbuf, l);
			}
		}
	}
	else if (type == 3)
	{
		RADIx(rbuf, l);
	}

}


void place(pSort::dataType* rbuf, int place_val, int c)
{
	    	pSort::dataType* temp = new pSort::dataType[c + 1];
			int* tmp = new int[10]; int neg=0 ,ng = 0;
			for (int i = 0; i < 10; i++)  //arr of place values
			{
				tmp[i] = 0;
				//tmp1[i] = 0;
			}
			for (int i = 0; i < c; i++)		//arr of values
			{
				temp[i].key = i;
			}

			int strt[size];			//begining index
			int count[size];			//no. of items to sort
			int end[size];				//ending index
			int cc = (int)(c / size);			//count of array elements to be sent to each thrd
			int rem = (int)(c % size);			//number of processes to send count+1 elements

			if (rem == 0)
			{
				for (int i = 0; i < size; i++)
				{
					count[i] = cc;
					strt[i] = i * cc;
					end[i] = strt[i] + cc-1;
				}
			}

			if (rem != 0)
			{
				for (int i = 0; i < rem; i++)
				{
					strt[i] = i * (cc + 1);
					count[i] = cc + 1;
					end[i] = strt[i] + count[i] - 1;
				}
				
				for (int i = rem; i < size; i++)
				{
					strt[i] = (i * cc) + rem;
					count[i] = cc;
					end[i] = strt[i] + count[i] - 1;
					
				}
			}
#pragma omp parallel   
			{
				int neg1 = 0;   //count of neg numbers with each thread
				int* tmp1 = new int[10];  //pvt arr of personal values 
				for (int i = 0; i < 10; i++)
				{
					tmp1[i] = 0;
				}
				
				for (int i = strt[omp_get_thread_num()]; i <= end[omp_get_thread_num()]; i++)
				{
					if (rbuf[i].key < 0)
					{
						neg1++;
					}
					else 
					{
						tmp1[(rbuf[i].key / place_val) % 10] += 1;  //record number of occurences of digit given by the exp
					}
				}
				tmp1[0] += neg1;  //0th index to strt after neg numbrs 
				for (int i = 1; i < 10; i++)
				{
					tmp1[i] += tmp1[i - 1];   //displacement of digit i, its offset

				}

#pragma omp critical(sec)
				{
					neg += neg1; ng = neg;
					for (int i = 0; i < 10; i++)
					{
						tmp[i] += tmp1[i];
					}
				}
				 
#pragma omp barrier
#pragma omp single
				{
					
					for (int i = c - 1; i >= 0; i--)
					{
						if (rbuf[i].key < 0)
						{
							int x = neg - 1; neg--;
							std::memcpy(temp + x, rbuf + i, sizeof(rbuf));
						}
						else 
						{
							int x = tmp[(rbuf[i].key / place_val) % 10] - 1;
							std::memcpy(temp + x, rbuf + i, sizeof(rbuf));
							tmp[(rbuf[i].key / place_val) % 10] -= 1;
						}
					}
					if (place_val == 1000000000)
					{
						QUICk(temp, 0, ng);
					}
				}

			}
			
			for (int i = 0; i <= c; i++)
			{
				std::memcpy(rbuf + i, temp + i, sizeof(rbuf));
			}
			
}

void RADIx(pSort::dataType* rbuf, int c)
{
	for (int i = 0; i < 10; i++)	  //10 because the range of 32-bit int allows these many digits
	{
		place(rbuf, pow(10, i), c );
	}

}


pSort::dataType* slice(pSort::dataType* rbuf, int start, int end)
{
	pSort::dataType* result = new pSort::dataType[(end - start)];
	
	int i; 

		for (i = start; i < end; i++)
		{
			result[i - start] = rbuf[i];
		}
	
	return result;
}

void merge(pSort::dataType* result, pSort::dataType* left, pSort::dataType* right, int leftLen, int rightLen)
{
	int i = 0, j = 0;
	while (i < leftLen && j < rightLen)
	{
		if (left[i].key < right[j].key)
		{
			result[i + j] = left[i];
			i++;
		}
		else
		{
			result[i + j] = right[j];
			j++;
		}
	}

	for (; i < leftLen; i++)
	{
		result[i + j] = left[i];
	}
	for (; j < rightLen; j++)
	{
		result[i + j] = right[j];
	}

}

void MERGe(pSort::dataType* rbuf, int len)
{
	if (len <= 1)
	{
		return;
	}
	pSort::dataType* left = slice(rbuf, 0, len / 2 + 1);
	pSort::dataType* right = slice(rbuf, len / 2, len);

#pragma omp task untied
			{
				MERGe(left, len / 2);
			}
#pragma omp task untied
			{
				MERGe(right, len - (len / 2));
			}
#pragma omp taskwait
			merge(rbuf, left, right, len / 2, len - (len / 2));

}


int part(pSort::dataType* rbuf, int low, int high)
{
	int pivot = rbuf[high].key;    // pivot
	int i = (low - 1);  // Index of smaller element

	for (int j = low; j <= (high - 1); j++)
	{
		// If current element is smaller than or equal to pivot
		if (rbuf[j].key <= pivot)
		{
			i++;
			pSort::dataType t = rbuf[i];
			rbuf[i] = rbuf[j];
			rbuf[j] = t;
		}
	}
	pSort::dataType t = rbuf[i + 1];
	rbuf[i + 1] = rbuf[high];
	rbuf[high] = t;
	return (i + 1);
}

void QUICk(pSort::dataType* rbuf, int l, int h)
{
	if (l < h)
	{
		int p = part(rbuf, l, h);

#pragma omp task untied
			{
				QUICk(rbuf, l, p - 1);
			}
	
#pragma omp task untied
			{
				QUICk(rbuf, p + 1, h);
			}
	}
}

void pSort::close()
{
	return;
}
