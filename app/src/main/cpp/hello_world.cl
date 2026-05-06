/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

 __kernel void helloWorld(__global char* data){
     int index = get_global_id(0);
     data[index] = data[0];
     data[256] = '\n';
 }
