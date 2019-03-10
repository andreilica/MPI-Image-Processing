#!/bin/bash

make clean
rm -f *.pnm *.pgm
make

echo "=================BW================"

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_identity.pgm identity
if [[ $(diff in/lenna_bw.pgm lenna_bw_identity.pgm) ]]
then 
    echo "identity filter failed"
else
    echo "identity filter worked"
fi

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_smooth.pgm smooth
if [[ $(diff ref/lenna_bw_smooth.pgm lenna_bw_smooth.pgm) ]]
then 
    echo "smooth   filter failed"
else
    echo "smooth   filter worked"
fi

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_blur.pgm blur
if [[ $(diff ref/lenna_bw_blur.pgm lenna_bw_blur.pgm) ]]
then 
    echo "blur     filter failed"
else
    echo "blur     filter worked"
fi

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_sharpen.pgm sharpen
if [[ $(diff ref/lenna_bw_sharpen.pgm lenna_bw_sharpen.pgm) ]]
then 
    echo "sharpen  filter failed"
else
    echo "sharpen  filter worked"
fi

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_mean.pgm mean
if [[ $(diff ref/lenna_bw_mean.pgm lenna_bw_mean.pgm) ]]
then 
    echo "mean     filter failed"
else
    echo "mean     filter worked"
fi

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_emboss.pgm emboss
if [[ $(diff ref/lenna_bw_emboss.pgm lenna_bw_emboss.pgm) ]]
then 
    echo "emboss   filter failed"
else
    echo "emboss   filter worked"
fi

mpirun -np $1 homework in/lenna_bw.pgm lenna_bw_bssembssem.pgm blur smooth sharpen emboss mean blur smooth sharpen emboss mean
if [[ $(diff ref/lenna_bw_bssembssem.pgm lenna_bw_bssembssem.pgm) ]]
then 
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss" 
    echo "mean" 
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss"
    echo "mean     filter failed"
else
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss" 
    echo "mean" 
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss"
    echo "mean     filter worked"
fi

echo "==================================="

echo "===============COLOR==============="

mpirun -np $1 homework in/lenna_color.pnm lenna_color_identity.pnm identity
if [[ $(diff in/lenna_color.pnm lenna_color_identity.pnm) ]]
then 
    echo "identity filter failed"
else
    echo "identity filter worked"
fi

mpirun -np $1 homework in/lenna_color.pnm lenna_color_smooth.pnm smooth
if [[ $(diff ref/lenna_color_smooth.pnm lenna_color_smooth.pnm) ]]
then 
    echo "smooth   filter failed"
else
    echo "smooth   filter worked"
fi

mpirun -np $1 homework in/lenna_color.pnm lenna_color_blur.pnm blur
if [[ $(diff ref/lenna_color_blur.pnm lenna_color_blur.pnm) ]]
then 
    echo "blur     filter failed"
else
    echo "blur     filter worked"
fi

mpirun -np $1 homework in/lenna_color.pnm lenna_color_sharpen.pnm sharpen
if [[ $(diff ref/lenna_color_sharpen.pnm lenna_color_sharpen.pnm) ]]
then 
    echo "sharpen  filter failed"
else
    echo "sharpen  filter worked"
fi

mpirun -np $1 homework in/lenna_color.pnm lenna_color_mean.pnm mean
if [[ $(diff ref/lenna_color_mean.pnm lenna_color_mean.pnm) ]]
then 
    echo "mean     filter failed"
else
    echo "mean     filter worked"
fi

mpirun -np $1 homework in/lenna_color.pnm lenna_color_emboss.pnm emboss
if [[ $(diff ref/lenna_color_emboss.pnm lenna_color_emboss.pnm) ]]
then 
    echo "emboss   filter failed"
else
    echo "emboss   filter worked"
fi

mpirun -np $1 homework in/lenna_color.pnm lenna_color_bssembssem.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean
if [[ $(diff ref/lenna_color_bssembssem.pnm lenna_color_bssembssem.pnm) ]]
then 
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss" 
    echo "mean" 
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss"
    echo "mean     filter failed"
else
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss" 
    echo "mean" 
    echo "blur"
    echo "smooth"
    echo "sharpen"
    echo "emboss"
    echo "mean     filter worked"
fi

echo "==================================="
make clean
rm -f *.pnm *.pgm