/* 
This progranm is designed to split the whole velocity and density models into several sub-domains

Coordinate configuration of seismic data:
 is=iz*ny*nx+iy*nx+ix;
 The fastest dim:        *.x
 The second fastest dim: *.y
 The slowest dim:        *.z
 
 Acknowledgement:

   Copyright (C) 2019 China University of Petroleum, Beijing
   Copyright (C) 2019 Ning Wang

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details: http://www.gnu.org/licenses/

*/

#include"stdio.h"
#include"stdlib.h"
#include <string.h>
#define pml 15            //the number of PML layers
#define nz 101            // z- grid number of model(without PML)
#define ny 101            // y- grid number of model(without PML)
#define nx 101            // x- grid number of model(without PML)
#define numpx 2           //the number of sub-domains at x- direction
#define numpy 2           //the number of sub-domains at y- direction

void num2str(char asc[6],int num);

int main()
{
    int sizex[numpx]; //arrays for storing the number of grid points (with pml) for sub-domains at x- direction
    int xef[numpx];   //arrays for storing the number of grid points (without pml) for sub-domains at x- direction
	int sizey[numpy]; //arrays for storing the number of grid points (with pml) for sub-domains at y- direction
    int yef[numpy];   //arrays for storing the number of grid points (without pml) for sub-domains at y- direction
    int modx;         //the remainder of the number of whole grid points at x- axis(with pml) divided by the number of sub-domains at x- direction
	int avex;    
    int mody;         //the remainder of the number of whole grid points at y- axis(with pml) divided by the number of sub-domains at y- direction
	int	avey;

	
    int nxp=nx+2*pml; // x- grid number of model(with PML)
    modx=nxp%numpx;
    avex=(nxp-modx)/numpx;

	
    int nyp=ny+2*pml; // y- grid number of model(with PML)
    mody=nyp%numpy;
    avey=(nyp-mody)/numpy;

    int ix,iy,iz,i,j,is;
    int offset_y,offset_x;

	//define the number of grid-points for every sub-domain at x- direction 
    for(i=0;i<numpx;i++)
    {
        sizex[i]=avex;
        if(i<modx){sizex[i]=sizex[i]+1;}

        xef[i]=sizex[i];
        if(i==0){xef[i]=xef[i]-pml;}
        if(i==numpx-1){xef[i]=xef[i]-pml;}

        printf("i=%d size=%d\n",i,xef[i]);

    }

	//define the number of grid-points for every sub-domain at y- direction 
    for(j=0;j<numpy;j++)
    {
        sizey[j]=avey;
        if(j<mody){sizey[j]=sizey[j]+1;}

        yef[j]=sizey[j];
        if(j==0){yef[j]=yef[j]-pml;}
        if(j==numpy-1){yef[j]=yef[j]-pml;}

      printf("j=%d size=%d\n",j,yef[j]);

    }

   float *vp_whole;  // the whole p-velocity model
   float *vp_each;   // p-velociy models for sub-domains
   FILE *fp=fopen("vp.bin","rb");
   FILE *fpp1[numpy][numpx];
   char name[60],ascx[6],ascy[6];
   vp_whole=(float*)malloc(sizeof(float)*nx*ny*nz);


   float *den_whole;  // the whole density model
   float *den_each;   // density models for sub-domains
   FILE *fp2=fopen("rho.bin","rb");
   FILE *fpp2[numpy][numpx];
   char name2[60];
   den_whole=(float*)malloc(sizeof(float)*nx*ny*nz);


   float *vs_whole;  // the whole s-velocity model
   float *vs_each;   // s-velociy models for sub-domains
   FILE *fp3=fopen("vs.bin","rb");
   FILE *fpp3[numpy][numpx];
   char name3[60];
   vs_whole=(float*)malloc(sizeof(float)*nx*ny*nz);

	for(iz=0;iz<nz;iz++)
	{
		for(iy=0;iy<ny;iy++)
		{
			for(ix=0;ix<nx;ix++)
			{
			is=iz*ny*nx+iy*nx+ix;
			fread(&vp_whole[is],sizeof(float),1,fp);
			fread(&den_whole[is],sizeof(float),1,fp2);
			fread(&vs_whole[is],sizeof(float),1,fp3);
			}
		}
	}	
	 //spliting the whole velocity & density models into subdomains    
    offset_y=0;
        for(j=0;j<numpy;j++)
        {
         num2str(ascy,j);                  
         
		 offset_x=0;
		for(i=0;i<numpx;i++)
        {
         num2str(ascx,i);          
          sprintf(name,"x%s_vp_y%s.dat",ascx,ascy);
          fpp1[j][i]=fopen(name,"wb"); 
		  vp_each=(float*)malloc(sizeof(float)*xef[i]*yef[j]*nz);

          sprintf(name2,"x%s_rho_y%s.dat",ascx,ascy);
          fpp2[j][i]=fopen(name2,"wb"); 
		  den_each=(float*)malloc(sizeof(float)*xef[i]*yef[j]*nz);

          sprintf(name3,"x%s_vs_y%s.dat",ascx,ascy);
          fpp3[j][i]=fopen(name3,"wb"); 
		  vs_each=(float*)malloc(sizeof(float)*xef[i]*yef[j]*nz);
		 
         for(iz=0;iz<nz;iz++)
         
	       {
		    for(iy=0;iy<yef[j];iy++)
		      {
		        for(ix=0;ix<xef[i];ix++)
		        {
			       is=iz*yef[j]*xef[i]+iy*xef[i]+ix;
				   vp_each[is]=vp_whole[iz*ny*nx+(iy+offset_y)*nx+(ix+offset_x)];
			       fwrite(&vp_each[is],sizeof(float),1,fpp1[j][i]);

				   den_each[is]=den_whole[iz*ny*nx+(iy+offset_y)*nx+(ix+offset_x)];
			       fwrite(&den_each[is],sizeof(float),1,fpp2[j][i]);

				   vs_each[is]=vs_whole[iz*ny*nx+(iy+offset_y)*nx+(ix+offset_x)];
			       fwrite(&vs_each[is],sizeof(float),1,fpp3[j][i]);
			    }
		     }  
		   }

			offset_x=offset_x+xef[i];
		 } 
		 offset_y=offset_y+yef[j];  
		}

        for(j=0;j<numpy;j++)
        {

		    for(i=0;i<numpx;i++)
              {
			   fclose(fpp1[j][i]);
			   fclose(fpp2[j][i]);
			   fclose(fpp3[j][i]);
		      }
		}
  
    fclose(fp);fclose(fp2);fclose(fp3);
    free(vp_whole);free(vp_each);free(den_whole);free(den_each);free(vs_whole);free(vs_each);    

    return 0;
}

// ==========================================================
//  This subroutine is used for transfroming number to string
//  =========================================================
void num2str(char asc[6],int num)
{
  char asc1,asc2,asc3,asc4,asc5,asc6;
  asc6='\0';
  if(num<10)
  {
	asc5='0'; 
    asc4='0'; 
    asc3='0'; 
    asc2='0'; 
    asc1=(char)(num+48); 
  }
  if(num>=10&&num<=99)
  {
    asc5='0'; 
    asc4='0'; 
    asc3='0'; 
    asc2=(char)(num/10+48); 
    asc1=(char)(num-num/10*10+48); 
  }
  if(num>99&&num<=999)
  {
	asc5='0';                    
    asc4='0';                   
    asc3=(char)(num/100+48);     
    asc2=(char)(num%100/10+48);  
    asc1=(char)(num%10+48);      
  }
  if(num>=1000&&num<=9999)
  {
	asc5='0'; 
    asc4=(char)(num/1000+48);      
    asc3=(char)((num/100)%10+48);
    asc2=(char)((num/10)%10+48);  
    asc1=(char)(num%10+48);       

  }
  if(num>=10000&&num<=99999)
  {
	  asc5=(char)(num/10000+48); 
      asc4=(char)((num/1000)%10+48); 
      asc3=(char)((num/100)%10+48);
      asc2=(char)((num/10)%10+48); 
      asc1=(char)(num%10+48);        

  }
   asc[0]=asc5;  
   asc[1]=asc4;  
   asc[2]=asc3;  
   asc[3]=asc2;
   asc[4]=asc1;
   asc[5]=asc6;


   return;
}
