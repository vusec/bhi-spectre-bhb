/*
 * Thursday, September 9th 2021
 *
 * Enrico Barberis - e.barberis@vu.nl
 * Pietro Frigo - p.frigo@vu.nl
 * Marius Muench - m.muench@vu.nl
 * Herbert Bos - herbertb@cs.vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 *
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

#ifndef _EVICT_H_
#define _EVICT_H_

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "helper.h"

//Max possible ev set size
#define EV_SET_MAX 50000

typedef struct ev_set_t {
    uint32_t size;
    uint32_t front;
    uint32_t rear;
    uint8_t *ptr[EV_SET_MAX];
} ev_set_t;


void enqueue(ev_set_t *ev, uint8_t *ptr) {
    assert(ev->size < EV_SET_MAX);

    ev->ptr[ev->rear] = ptr;
    ev->size++;
    ev->rear = (ev->rear + 1) % EV_SET_MAX;
}

uint8_t* dequeue(ev_set_t *ev) {
    uint8_t *ret;
    assert(ev->size > 0);

    ret = ev->ptr[ev->front];
    ev->front = (ev->front + 1) % EV_SET_MAX;
    ev->size--;

    return ret;
}

void print_ev(ev_set_t *ev) {
    printf("ev_set size = %d\n", ev->size);
    printf("front %d\n", ev->front);
    printf("rear %d\n", ev->rear);

    int idx = ev->front;
    printf("{");
    for(int i=0; i<ev->size; i++) {
        printf("%p, ", ev->ptr[idx]);
        idx = (idx + 1) % EV_SET_MAX;
    }
    printf("}\n");
}

void evict(ev_set_t *ev) {
    int idx;

    //TODO loop unrool when minimal?
    for(int j=0; j<4; j++)
    {
        idx = ev->front;
        for(int i=0; i<ev->size; i++) {
            maccess(ev->ptr[idx]);
            idx = (idx + 1) % EV_SET_MAX;
        }

        idx = ev->rear-1;
        for(int i=0; i<ev->size; i++) {
            if(idx < 0) idx = EV_SET_MAX-1;
            maccess(ev->ptr[idx]);
            idx = (idx - 1);
        }
    }
}

#endif
