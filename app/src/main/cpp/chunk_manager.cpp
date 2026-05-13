#include "chunk_manager.h"
#include "buffer.h"
#include "chunk.h"

ChunkManager::ChunkManager(int n_world_width, int n_world_height, CL* cl) :
    world_width(n_world_width), 
    world_height(n_world_height)
{
    camera = {0,0,1};

    chunk_size = 128;

    render_buffer = init_render_buf(world_width,world_height,'?');

    chunks = std::vector<std::vector<Chunk>>();

    for (int y = 0; y < world_height; y+= chunk_size) {
        std::vector<Chunk> row = std::vector<Chunk>();
        for (int x = 0; x < world_width; x+= chunk_size) {
            row.push_back(
                Chunk(x / chunk_size, y / chunk_size,chunk_size,cl)
            );
        }
        chunks.push_back(row);
    }

    // Tell the kernel what size each chunk is for rendering
    cl->setArg(3,chunk_size,cl->render_kernel);
    cl->setArg(4,chunk_size,cl->process_kernel);

    // Create the GPU buffer for rendering and link it to the render kernel
    render_buffer_index = cl->makeRenderBuffer(render_buffer);
    cl->setArg(8,render_buffer,render_buffer_index,cl->render_kernel);
    // Link the render buffer to the process kernel too
    // cl->setArg(0, render_buffer,render_buffer_index,cl->process_kernel);
    
    // Buffer where (0,0),(0,1),(0,2),(0,3) = TL, TR, BL, BR diagonal chunks,
    // (1->chunk_size,0) = North, 
    // (1->chunk_size,1) = East,
    // (1->chunk_size,2) = South,
    // (1->chunk_size,3) = West,
    swap_requests = init_buf(chunk_size,4,0);
    swap_requests_index = cl->makeBuffer(swap_requests);
    cl->setArg(9,swap_requests,swap_requests_index,cl->process_kernel);
}

Chunk* ChunkManager::get_chunk(int x, int y) {
    if (y >= chunks.size() || y < 0 || x >= chunks.at(y).size() || x < 0) { return nullptr; }
    return &chunks.at(y).at(x);
}

buffer ChunkManager::get_render_buffer() {
    return render_buffer;
}

void ChunkManager::render_chunk(CL* cl,Chunk* chunk) {
    // Tell the kernel the chunk's data and the chunk's position
    cl->setArg(0,chunk->get_data(),chunk->buffer_index,cl->render_kernel);
    cl->setArg(1,chunk->chunk_x,cl->render_kernel);
    cl->setArg(2,chunk->chunk_y,cl->render_kernel);

    cl->setArg(4,camera.x,cl->render_kernel);
    cl->setArg(5,camera.y,cl->render_kernel);

    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk->get_data().data, chunk->buffer_index);
    cl->enqueueKernel(chunk_size,chunk_size,cl->render_kernel);
    cl->enqueueRenderReadBuffer(world_width,world_height, render_buffer.data, render_buffer_index);
}

void ChunkManager::update_chunks(CL* cl) {
    for (int y = 0; y < chunks.size(); y++) {
        for (int x = 0; x < chunks.at(y).size(); x++) {
        
        Chunk* chunk = get_chunk(x,y);

        char materials[] = {'S','W','R',' '};

        if (y == 1 && x == 1) {
            chunk->set_cell(chunk_size / 2,chunk_size / 2, materials[(y * chunks.size() + x)%2]);
        }
        refresh_chunk(cl,chunk);
        
        cl->setArg(0,chunk->get_data(),chunk->buffer_index,cl->process_kernel);
        cl->setArg(2,chunk->chunk_x,cl->process_kernel);
        cl->setArg(3,chunk->chunk_y,cl->process_kernel);

        for (int j = 0; j < chunk->highest_speed; j++) {
            // Create chunk_size/2 x chunk_size/2 tasks, each processing a 2x2 block.
            cl->setArg(1,chunk->get_iteration(),cl->process_kernel);
            cl->enqueueKernel(chunk_size / 2, chunk_size / 2, cl->process_kernel);

            chunk->iterate();

            process_swap_requests(cl,chunk);

        }

        cl->enqueueReadBuffer(chunk_size,chunk_size, chunk->get_data().data, chunk->buffer_index);
        render_chunk(cl,chunk);
        }
    }
}

void ChunkManager::refresh_chunk(CL* cl,Chunk* chunk) {
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk->get_data().data, chunk->buffer_index);
}

void ChunkManager::input(InputState input_state) {
    if (input_state.camera_up)   {camera.y  += camera.speed; }
    if (input_state.camera_right){camera.x  += camera.speed; }
    if (input_state.camera_down) {camera.y  -= camera.speed; }
    if (input_state.camera_left) {camera.x  -= camera.speed; }
}

void ChunkManager::process_swap_requests(CL* cl,Chunk* chunk) {
    cl->enqueueReadBuffer(chunk_size,4, swap_requests.data, swap_requests_index);

    // buffer_value tl = get_buffer(swap_requests,0,0);
    // buffer_value tr = get_buffer(swap_requests,0,1);
    // buffer_value bl = get_buffer(swap_requests,0,2);
    // buffer_value br = get_buffer(swap_requests,0,3);
    
    for (int side = 0; side < 4; side++) {
        for (int pos = 0; pos < swap_requests.width; pos++) {
            buffer_value bv = get_buffer(swap_requests, pos, side);
            if (bv.material != ' ' && bv.material != 0) {
                // std::cout << "Swap request for [" << bv.material << "] (" << bv.x << "," << bv.y << ")" << " At: " << pos << "," << side << std::endl;
                int x = chunk->chunk_x;
                int y = chunk->chunk_y;
                switch(side) {
                    case 0: y += 1; break;
                    case 1: x += 1; break;
                    case 2: y -= 1; break;
                    case 3: x -= 1; break;
                }
                Chunk* neighbour = get_chunk(x,y);
                if (neighbour) {
                    std::cout << chunk-> chunk_x << "," << chunk->chunk_y << " -> " <<neighbour->chunk_x << "," << neighbour->chunk_y << std::endl;
                    buffer_value nv = {(char)pos,0,'S'}; //get_buffer(neighbour->get_data(),pos,0);
                    set_buffer(chunk->get_data(),pos,chunk_size-1,nv);
                    set_buffer(neighbour->get_data(),pos,0,bv);
                }
            }
        }
    }

    // cl->enqueueWriteBuffer(chunk_size,chunk_size, chunk->get_data().data, chunk->buffer_index);
    // if (neighbour) cl->enqueueWriteBuffer(chunk_size,chunk_size, neighbour->get_data().data, neighbour->buffer_index);

}
