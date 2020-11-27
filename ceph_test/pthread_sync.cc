#include <atomic>
#include <iostream>
#include <pthread.h>

std::atomic<int> test;

void* thread1(void* t){

    pthread_t tid = pthread_self();
    printf("Thread Before: %d\n",tid);
    test++;
    return NULL;
}

void* thread2(void* t){
  
  while(test.load( std::memory_order_acquire ) < 3);
  printf("Thread End\n");
  return NULL;

}

int main(int argc,char *argv[]){
  
  pthread_t threads[4];
  test.store(0,std::memory_order_release);
  pthread_create(&threads[0],NULL,thread2,NULL);
  for (int i=1;i<4;++i){
    int r = pthread_create(&threads[i],NULL,thread1,NULL);
  }

  for (int i=0;i<4;++i){
    pthread_join(threads[i],NULL);
  }

}