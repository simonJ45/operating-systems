/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){

	int i = leftstart;
    int j = rightstart;
    int k = leftstart;

    // Step 1: Compare elements in both subarrays
    while (i <= leftend && j <= rightend) {

        if (A[i] <= A[j]) {
            B[k] = A[i];
            i++;
        } else {
            B[k] = A[j];
            j++;
        }

        k++;
    }

    // Step 2: Copy remaining elements from left subarray
    while (i <= leftend) {
        B[k] = A[i];
        i++;
        k++;
    }

    // Step 3: Copy remaining elements from right subarray
    while (j <= rightend) {
        B[k] = A[j];
        j++;
        k++;
    }

    // Step 4: Copy sorted elements back into A
    memcpy(A + leftstart,
           B + leftstart,
           (rightend - leftstart + 1) * sizeof(int));
}


/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){

	int mid;

	// Base case: zero or one element
    if (left >= right) {
        return;
    }

    mid = left + (right - left) / 2;

    // Sort the two halves
    my_mergesort(left, mid);
    my_mergesort(mid + 1, right);

    // Merge the sorted halves
    merge(left, mid, mid + 1, right);
}



/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
	
	struct argument *current;

    int left;
    int right;
    int level;
    int mid;

    pthread_t t1;
    pthread_t t2;

    struct argument *leftArg;
    struct argument *rightArg;

    int result1;
    int result2;

    // Read arguments
    if (arg == NULL) {
        return NULL;
    }

    current = (struct argument *)arg;

    left = current->left;
    right = current->right;
    level = current->level;

    // Base case: zero or one element
    if (left >= right) {
        return NULL;
    }

    // Stop creating threads when cutoff is reached
    if (level >= cutoff) {
        my_mergesort(left, right);
        return NULL;
    }

    // Divide the array into two halves
    mid = left + (right - left) / 2;

    // Prepare arguments for child threads
    leftArg = buildArgs(left, mid, level + 1);
    rightArg = buildArgs(mid + 1, right, level + 1);

    // If allocation fails, use serial merge sort
    if (leftArg == NULL || rightArg == NULL) {
        free(leftArg);
        free(rightArg);

        my_mergesort(left, right);
        return NULL;
    }

    // Create thread for left half
    result1 = pthread_create(&t1, NULL,
                             parallel_mergesort, leftArg);

    // If thread creation fails, sort left half serially
    if (result1 != 0) {
        my_mergesort(left, mid);
    }

    // Create thread for right half
    result2 = pthread_create(&t2, NULL,
                             parallel_mergesort, rightArg);

    // If thread creation fails, sort right half serially
    if (result2 != 0) {
        my_mergesort(mid + 1, right);
    }

    // Wait for successfully created threads
    if (result1 == 0) {
        if (pthread_join(t1, NULL) != 0) {
            fprintf(stderr, "Failed to join left thread\n");
            exit(EXIT_FAILURE);
        }
    }

    if (result2 == 0) {
        if (pthread_join(t2, NULL) != 0) {
            fprintf(stderr, "Failed to join right thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Both halves are sorted, so merge them
    merge(left, mid, mid + 1, right);

    // Release memory allocated for child arguments
    free(leftArg);
    free(rightArg);

    return NULL;
}
		

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){

	struct argument *newArg;

    newArg = malloc(sizeof(struct argument));

    // Check memory allocation
    if (newArg == NULL) {
        fprintf(stderr, "Failed to allocate thread arguments\n");
        return NULL;
    }

    newArg->left = left;
    newArg->right = right;
    newArg->level = level;

    return newArg;
}


