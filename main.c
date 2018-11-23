/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: nicolas
 *
 * Created on 25 septembre 2018, 15:00
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * 
 */



typedef struct ByteArray{
    unsigned char values[4][4];
    //char ** values;
};

void showArray(unsigned char values[4][4], char str[]){
    int i,j = 0;
    printf(str);
    printf(":\n");
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            printf("%hhx\t", values[i][j]);
        }
        printf("\n");
    }
    printf("===========================\n");
}

void showSBox(unsigned char values[16][16]){
    int i,j = 0;
    for(i = 0; i < 16; i++){
        for(j = 0; j < 16; j++){
            printf("%#04x\t", values[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

char xor (char op1, char op2){
    return op1 ^ op2;
}

void readFromFile(struct ByteArray * in, char * fileName){
    FILE * file;
    char data;
    int i = 0;
    int j = 0;
    file = fopen(fileName, "r");
    if(file == NULL){
        printf("Erreur lors de l'ouverture du fichier %s\n", fileName);
        printf("Fin du programme\n");
        exit(1);
    }
    
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            fread(&data, sizeof(char), 1, file);
            //printf("%d\t", data);
            in->values[j][i] = data;
        }
        //printf("\n");
    }
    fclose(file);
}

void shiftOneRow(char row[4], int n){
    int i = 0;
    int j = 0;
    char savedValue;
    for(i = 0; i < n; i++){
        savedValue = row[0];
        for(j = 0; j < 4; j++){
            row[j] = row[(j+1)%4];
        }
        row[3] = savedValue;
    }
}

void shiftRows(struct ByteArray* state){
    int i = 0;
    for(i = 0; i < 4; i++){
        shiftOneRow(state->values[i], i);
    }
}

void addRoudKey(struct ByteArray * state, struct ByteArray * key){
    int i = 0;
    int j = 0;
    for(i = 0; i < 4 ; i++){
        for (j = 0; j < 4 ; j++){
            state->values[i][j] = xor(state->values[i][j], key->values[i][j]);
        }
    }
}

void subBytes(struct ByteArray * state, unsigned char sBox[16][16]){
    int i = 0;
    int j = 0;
    int col = 0;
    int row = 0;
    for(i = 0; i < 4 ; i++){
        for (j = 0; j < 4 ; j++){
            col = (int) state->values[i][j] & 0x0f;
            row = (int) (state->values[i][j] & 0xf0)>>4;
            state->values[i][j] = sBox[row][col];
            //printf("\n");
        }
    }
}

unsigned char GMul(unsigned char a, unsigned char b)
{ // Galois Field (256) Multiplication of two Bytes
    /*unsigned char p = 0;
    int counter;
    for (counter = 0; counter < 8; counter++)
    {
        if ((b & 1) != 0)
        {
            p ^= a;
        }
        int hi_bit_set = (a & 0x80) != 0;
        a <<= 1;
        if (hi_bit_set) // == vrai
        {
            a ^= 0x1B; // x^8 + x^4 + x^3 + x + 1 
        }
        b >>= 1;
    }
    return p;*/
    int i;
    int retval= 0;
     
    /* GF multiplication with insane bit shifting */
    for(i = 0; i < 8; i++) {
        if((b & 1) == 1) 
            retval ^= a;
         
        if((a & 0x80) == 0x80) {
            a <<= 1;
            a  ^= 0x1b;
        } else {
            a <<= 1;
        }
         
        b >>= 1;
    }
     
    return (unsigned char)retval;
}

void mixColumns(struct ByteArray * state){
    unsigned char prevState[4][4];
    int i = 0;
    int j = 0;
    for (i=0; i<4; i++){
        for (j=0; j<4; j++){
            prevState[i][j] = state->values[i][j];
        }
    }
    //showArray(prevState,"prevState");
    for (i=0; i<4; i++){
        state->values[0][i] = GMul(2, prevState[0][i]) ^ GMul(3, prevState[1][i]) ^ prevState[2][i] ^ prevState[3][i];
        state->values[1][i] = prevState[0][i] ^ GMul(2, prevState[1][i]) ^ GMul(3, prevState[2][i]) ^ prevState[3][i];
        state->values[2][i] = prevState[0][i] ^ prevState[1][i] ^ GMul(2, prevState[2][i]) ^ GMul(3, prevState[3][i]);
        state->values[3][i] = GMul(3, prevState[0][i]) ^ prevState[1][i] ^ prevState[2][i] ^ GMul(2, prevState[3][i]);
    }
    
    
    //dupliquer state
    //faire les calculs avec variable locale et mettre les resultats dans state
}

void getSBox(unsigned char sBox[16][16]){
    FILE * file;
    char * fileName = "SBox.bin";
    char data;
    int i = 0;
    int j = 0;
    file = fopen(fileName, "r");
    if(file == NULL){
        printf("Erreur lors de l'ouverture du fichier %s\n", fileName);
        printf("Fin du programme\n");
        exit(1);
    }
    
    for(i = 0; i < 16; i++){
        for(j = 0; j < 16; j++){
            fread(&data, sizeof(char), 1, file);
            //printf("%d\t", data);
            sBox[i][j] = data;
        }
        //printf("\n");
    }
    fclose(file);
    
}


int main(int argc, char** argv) {
    
    struct ByteArray * input = (struct ByteArray*)malloc(sizeof(struct ByteArray));
    struct ByteArray * key = (struct ByteArray*)malloc(sizeof(struct ByteArray));
    int i, j = 0;
    unsigned char sBox[16][16];
    readFromFile(input, "input.bin");
    readFromFile(key, "key.bin");
    getSBox(sBox);
    //showSBox(sBox);
    showArray(input->values, "input");
    showArray(key, "key");
    
    addRoudKey(input, key);
    showArray(input->values, "after addRoundKey");
    subBytes(input, sBox);
    showArray(input->values, "after subBytes");
    shiftRows(input);
    showArray(input->values, "after shiftRows");
    mixColumns(input);
    showArray(input->values, "after mixColumns");
    //showArray(input->values);
    /*
    unsigned char a = 127;
    unsigned char b = 98;
    printf("%hhx\t", GMul(a,b));
     */

    return (EXIT_SUCCESS);
}

