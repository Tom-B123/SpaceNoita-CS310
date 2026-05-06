/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

 __kernel void helloWorld(__global char* data){
     for (int i = 1; i < 12; i++) {
         data[i] = data[i-1];
     }
     data[12] = '\n';
 }
