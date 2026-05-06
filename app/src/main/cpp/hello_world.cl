/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

 __kernel void helloWorld(__global char* data){
     int index = get_global_id(0);
     data[index+1] = data[0];
 }
