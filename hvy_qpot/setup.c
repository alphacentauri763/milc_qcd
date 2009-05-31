/******** setup.c *********/
/* MIMD version 6 */
/* original code by UMH */
/* 2/19/98 Version 5 port CD */

#define IF_OK if(status==0)

#include "hvy_qpot_includes.h"
#include <string.h>

/* Each node has a params structure for passing simulation parameters */
#include "params.h"
params par_buf;

int  setup()   {
int initial_set();
int prompt;

        /* print banner, get volume, nflavors, seed */
    prompt=initial_set();
        /* initialize the node random number generator */
    setup_layout();
        /* allocate space for lattice, set up coordinate fields */
    make_lattice();
        /* set up neighbor pointers and comlink structures */
    make_nn_gathers();

    return(prompt);
}


/* SETUP ROUTINES */
int initial_set(){
  int prompt,status;

    /* On node zero, read lattice size, seed, nflavors and send to others */
    if(mynode()==0){
        /* print banner */
        printf("Pure gauge SU3 Wilson loops\n");
        printf("MIMD version 6\n");
        printf("Machine = %s, with %d nodes\n",machine_type(),numnodes());
	time_stamp("start");

        status=get_prompt(stdin, &prompt);
        IF_OK status += get_i(stdin, prompt,"nx",&par_buf.nx);
        IF_OK status += get_i(stdin, prompt,"ny",&par_buf.ny);
        IF_OK status += get_i(stdin, prompt,"nz",&par_buf.nz);
        IF_OK status += get_i(stdin, prompt,"nt",&par_buf.nt);

	if(status>0) par_buf.stopflag=1; else par_buf.stopflag=0;
    } /* end if(mynode()==0) */

    /* Node 0 broadcasts parameter buffer to all other nodes */
    broadcast_bytes((char *)&par_buf,sizeof(par_buf));

    if( par_buf.stopflag != 0 )
      normal_exit(0);

    nx=par_buf.nx;
    ny=par_buf.ny;
    nz=par_buf.nz;
    nt=par_buf.nt;
    
    this_node = mynode();
    number_of_nodes = numnodes();
    volume=nx*ny*nz*nt;
    return(prompt);
}

/* read in parameters and coupling constants    */
int readin(int prompt){
/* read in parameters for su3 monte carlo       */
/* argument "prompt" is 1 if prompts are to be given for input  */

int status,i;
Real x;
char savebuf[20];
double dtime;

    /* On node zero, read parameters and send to all other nodes */
    if(this_node==0){

        printf("\n\n");
	status = 0;
    
	/* smearing iterations and factor */
	IF_OK status += get_i(stdin, prompt,"no_smear_level", 
			      &par_buf.no_smear_level);
	IF_OK status += get_i(stdin, prompt,"smear_num[0]",&par_buf.smear_num[0]);
	IF_OK status += get_i(stdin, prompt,"smear_num[1]",&par_buf.smear_num[1]);
	IF_OK status += get_i(stdin, prompt,"smear_num[2]",&par_buf.smear_num[2]);
	IF_OK status += get_i(stdin, prompt,"smear_num[3]",&par_buf.smear_num[3]);
	IF_OK status += get_i(stdin, prompt,"smear_num[4]",&par_buf.smear_num[4]);

	IF_OK status += get_f(stdin, prompt,"smear_fac",&par_buf.smear_fac);

	/* off_axis_flag : do off-axis Wilson loops? */
	IF_OK status += get_i(stdin, prompt,"off_axis_flag",&par_buf.off_axis_flag);
    
	/* find out what kind of starting lattice to use */
	IF_OK status += ask_starting_lattice(stdin,  prompt, &(par_buf.startflag),
	    par_buf.startfile );

	/* find out what to do with lattice at end */
	IF_OK status += ask_ending_lattice(stdin,  prompt, &(par_buf.saveflag),
	    par_buf.savefile );
	IF_OK status += ask_ildg_LFN(stdin,  prompt, par_buf.saveflag,
				      par_buf.stringLFN );

	if( status > 0)par_buf.stopflag=1; else par_buf.stopflag=0;
    } /* end if(this_node==0) */

    /* Node 0 broadcasts parameter buffer to all other nodes */
    broadcast_bytes((char *)&par_buf,sizeof(par_buf));

    if( par_buf.stopflag != 0 )
      normal_exit(0);


	no_smear_level = par_buf.no_smear_level;
	smear_num[0] = par_buf.smear_num[0];
	smear_num[1] = par_buf.smear_num[1];
	smear_num[2] = par_buf.smear_num[2];
	smear_num[3] = par_buf.smear_num[3];
	smear_num[4] = par_buf.smear_num[4];
	smear_fac = par_buf.smear_fac;
	startflag = par_buf.startflag;
	off_axis_flag = par_buf.off_axis_flag;
	saveflag = par_buf.saveflag;
	strcpy(startfile,par_buf.startfile);
	strcpy(savefile,par_buf.savefile);
	strcpy(stringLFN, par_buf.stringLFN);

    /* Do whatever is needed to get lattice */
	if( startflag != CONTINUE )
	  startlat_p = reload_lattice( startflag, startfile );

    return(0);
}
