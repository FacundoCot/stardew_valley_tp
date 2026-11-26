#ifndef CELL_POS_H
#define CELL_POS_H

struct CellPos {
    int x;
    int y;

    //definimos este operador porque lo vamos a usar como clave de
    //map, por lo tanto necesitamos que lo pueda ordenar
    bool operator<(const CellPos& other) const;
};

#endif