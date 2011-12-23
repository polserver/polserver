///*
//History
//=======
//
//Notes
//=======
//
//*/
//
//#include "stl_inc.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <search.h>
//
//#include "rarray.h"
//
//GenRArray::GenRArray(
//    int ilimit, int idelta)
//{
//     ar = NULL;
//     limit = 0;
//     delta = idelta;
//     nitems = 0;
//     resizeto(ilimit);
//}
//
//GenRArray::~GenRArray()
//{
//    flush();
//    if (ar) free(ar);
//    ar = NULL;
//}
//
//void GenRArray::destroy(void *data)
//{
//    free(data);
//}
//
//void GenRArray::flush(void)
//{
//    while(nitems) kill(nitems-1);
//}
//void GenRArray::resizeto(unsigned newlimit)
//{
//    if (newlimit >= (int)((65536uL-16) / sizeof(void *))) return;
//    if (newlimit<0 || newlimit < nitems) newlimit = nitems;
//    size_t size = sizeof(void *) * (newlimit);
//    void **nar = (void **) realloc( (void *) ar, size);
//    if (nar) {
//        if (!ar) 
//			memset(nar,0,size);
//		else
//			memset(nar + nitems, 0, (newlimit-nitems)*sizeof(void *) );
//        ar = nar;
//        limit = newlimit;
//    }
//}
//
//int GenRArray::insert(void *ptr, unsigned x)
//{
//    if ((nitems+1) > limit) resizeto(limit + delta);
//
//    if ((nitems+1) > limit) return -1;
//    if (x > nitems+1) return -1;
//
//    memmove(&ar[x+1], &ar[x], (nitems-x) * sizeof(void *) );
//    nitems++;
//    ar[x] = ptr;
//    return 0;
//}
//
//int GenRArray::setat(void *ptr, unsigned x)
//{
//    if ((x+1) > limit) 
//		resizeto(x+1);
//
//    if ((x+1) > limit) 
//		return -1;
//
//    if ((x+1) > nitems) 
//		nitems = x+1;
//
//    ar[x] = ptr;
//    return 0;
//}
//
//int GenRArray::remove(unsigned x)
//{
//    if (x >= nitems) return -1;
//    nitems--;
//    memmove(&ar[x],&ar[x+1],(nitems-x+1)*sizeof(void *));
//    return 0;
//}
//
//const void *GenRArray::retrieve(unsigned position) const
//{
//    if (position >= nitems) return NULL;
//    return ar[position];
//}
//
//void *GenRArray::retrieve(unsigned position)
//{
//    if (position >= nitems) return NULL;
//    return ar[position];
//}
//
//void *GenRArray::get(unsigned position)
//{
//    void *data = NULL;
//    if ((data = retrieve(position)) != NULL) {
//        remove(position);
//        return data;
//    }
//    return NULL;
//}
//
//int GenRArray::find(void *data, unsigned *which)
//{
//    for(unsigned i=0;i<nitems;i++) {
//        if (ar[i] == data) {
//            if (which) *which = i;
//            return 1;
//        }
//    }
//    return 0;
//}
//
//int GenRArray::kill(unsigned position)
//{
//    void *d = retrieve(position);
//    if (d) destroy(d);
//    return remove(position);
//}
//
//int (*comp2)(void *a, void *b);
//
//int comp1(void **a, void **b)
//{
//    return (*comp2)(*a,*b);
//}
//
//void GenRArray::sort(int (*comp)(void *a, void *b))
//{
//    comp2 = comp;
//    qsort(ar,nitems,sizeof(void *),
//        (int (*)(const void *, const void *)) comp1);
//}
//
//int GenRArray::read(FILE *fp, size_t size)
//{
//    if (!fp) return -1;
//    while (!feof(fp)) {
//        void *n = malloc(size);
//        if (n==NULL) return -2;
//        if (::fread(n,size,1,fp) != 1) {
//            free(n);
//            return 0;
//        }
//        append(n);
//    }
//    return 0;
//}
//
//int GenRArray::read(unsigned char *filename, size_t size)
//{
//    int res = -1;
//    FILE *fp = fopen((char *) filename,"rb");
//    if (fp) {
//        res = read(fp,size);
//        fclose(fp);
//    }
//    return res;
//}
//
//int GenRArray::write(FILE *fp, size_t size)
//{
//    if (!fp) return -1;
//    void *n;
//    for(unsigned c = 0; c < nitems; c++) {
//        n = ar[c];
//        ::fwrite(n,size,1,fp);
//    }
//    return 0;
//}
//
//int GenRArray::write(unsigned char *filename, size_t size)
//{
//    FILE *fp = fopen((char *) filename,"wb");
//    int res = write(fp,size);
//    fclose(fp);
//    return res;
//}
//
//void *GenRArray::get(void *target, int (*f)(void *, void *))
//{
//    void *data;
//    unsigned i;
//    for(i=0;i<nitems;i++) {
//        data = retrieve(i);
//        if (f(target, data)==0) {
//            remove(i);
//            return data;
//        }
//    }
//    return NULL;
//}
//
//void GenRArray::traverse(void (*f)(void *data, void *staticdata), void *sdata)
//{
//    for(unsigned int i = 0; i < nitems; i++)
//      if (ar[i]) (*f)(ar[i], sdata);
//}
