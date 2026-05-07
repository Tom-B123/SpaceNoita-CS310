/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

 __kernel void helloWorld(__global char* data,int iteration, 
        int width, int height, int n_width, int n_height){
     int index = get_global_id(0);

    if (index == (iteration/10) % (width*n_width)) {
        data[index] = '@';
        return;
    }

     /* for (int i = 0; i < n_spawners; i++) { */
     /*     if (index == spawners[i]) { */
     /*         data[index] = '@'; */
     /*         return; */
     /*     } */
     /* } */

     int x = index % ((width + 1)/n_width);
     int y = index / ((width + 1)/n_height);

     x = (x * n_width) + iteration%n_width;
     y = (y * n_height) + iteration%n_height;

     int index1 = y * width + x;
     int index2 = y * width + x+1;
     int index3 = (y+1) * width + x;
     int index4 = (y+1) * width + x+1;

    if (y < height-1) {
        if (data[index1] == '@' && data[index3] != '@') {
            char tmp = data[index1];
            data[index1] = data[index3];
            data[index3] = tmp;
        }
        if (data[index2] == '@' && data[index4] != '@') {
            char tmp = data[index2];
            data[index2] = data[index4];
            data[index4] = tmp;
        }
        if (x < width - 1 && x > 0) {
            if (data[index1] == '@' && data[index3] == '@'&& data[index4] != '@') {
                char tmp = data[index1];
                data[index1] = data[index4];
                data[index4] = tmp;
            }
            if (data[index2] == '@' && data[index4] == '@'&& data[index3] != '@') {
                char tmp = data[index3];
                data[index3] = data[index2];
                data[index2] = tmp;
            }
        }
    }
 }
