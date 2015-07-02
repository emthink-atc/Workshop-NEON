// ---- Include Files ---- //

#include <stdio.h>
#include "xvid.h"
#include "avilib_mem.h"
#include "xvid_if.h"
#include "clcd.h"
#include "avifile.h"
#include "pmu.h"
#include "processor.h"
#include "platform.h"
#include "semi.h"
#include "ple.h"
#include "arm_neon.h"

// ---- Local Defines ---- //
#define FRAMES_PLAYBACK 20

#define TINY  0
#define SMALL  1
#define MEDIUM 2
#define LARGE  3

#define AVI_FILE LARGE



#define SEMIHOST 1

// ---- Global Variables ---- //

static char working_buffer[1024*1024];  // Input data for XVID decoder    
extern screen_t screen;                 // Frame Buffer structure  

int main(void) {

    // XVID codec
    Xvid_info movie;
    xvid_dec_stats_t xvid_dec_stats;
    uint16_t *video_pointer;   // Pointer to video memory
    uint32_t key_frame,status;
    uint8_t *mp4_ptr=NULL;     // Current position in working buffer
    int bytes_buffered=0;      // Bytes in working_buffer
    int bytes_processed=0;     // Bytes processed in working_buffer
    
    // Statistics
    char desc[500]; 
    unsigned int dec_ticks,load_ticks,frames=0;
    unsigned long long totdec_ticks=0,totload_ticks=0;
    double avgdec_ticks, avgload_ticks;
 
    // Which platform and cpu architecture
    int arch;
    char architecture_desc[50];
    char cpu_desc[50];
    int platform;
    char platform_desc[50];
    int counters;
    char counters_desc[50];

    // PRACTICAL 9.1.2
    // ---------------
    
    // TODO: Define a new NEON vector to hold two 32-bit unsigned integer values.
    // This will hold the frame totals.
    
    // Note: Use the following NEON data type:
    // uint32x2_t - Two unsigned 32-bit integers, packed into a 64bit "D" register 

    uint32x2_t neon_ticks;
    
    // TODO: Define a new NEON vector to hold two 64-bit unsigned integer values.
    // This will hold the accumalated totals after each frame.
    
    // Note: Use the following NEON data type:
    // uint64x2_t - Two unsigned 64-bit integers, packed into a 128bit "Q" register

    uint64x2_t neon_totals;

    // TODO: Initialize the uint64x2_t variable defined above to zero for each element
    // Use the vdupq_n_u64() intrinsic.
    
    neon_totals = vdupq_n_u64(0);

    // ---------------

    
    platform = whichPlatform(platform_desc);
    arch = whichArchitecture(architecture_desc); 
    whichProcessor(cpu_desc);
    
    // Enable processor features
    if(arch == ARCHV6) 
    {
        v6CPUFeaturesEnable();
        sprintf(desc,"No PMU counters available\n");
    }
    if(arch == ARCHV7) 
    {
        v7CPUFeaturesEnable();
    }
    
    // Enable baseboard features
    if (platform == EB_BASEBOARD)          CLCD_Init_EB();
    else if (platform == PB1176_BASEBOARD) CLCD_Init_PB1176();
    else if (platform == PBA8_BASEBOARD)   CLCD_Init_PBA8();
    else if (platform == PLATFORM_UNKNOWN) CLCD_Dummy();
    else CLCD_Dummy();
    
    // Check if RTSM model supports cycle counters
    counters = isCountersWorking(counters_desc, arch);



    // Create Info screen
    if(counters==COUNTERS_OK)
    {
        sprintf(desc,"XVID benchmark:\n  Platform: %s\n  Architecture: %s (%s)\n  Cycle counters: %s\n  Generating statistics ...\n",platform_desc,architecture_desc,cpu_desc,counters_desc);
    }
    else
    {
        sprintf(desc,"XVID benchmark:\n  Platform: %s\n  Architecture: %s (%s)\n  Cycle counters: %s\n  No statistics available\n",platform_desc,architecture_desc,cpu_desc,counters_desc);
    }
    
    
#if(SEMIHOST)
        semiPrint(desc);
#endif        
    
    // Show on back frame buffer
    CLCD_Block(screen.back,0,400,640,80,BLACK);
    CLCD_Rect(screen.back,0,400,640,80,WHITE);
       CLCD_Text(screen.back,5,405,WHITE,desc);    
    
       // Show on front frame buffer
    CLCD_Block(screen.front,0,400,640,80,BLACK);
    CLCD_Rect(screen.front,0,400,640,80,WHITE);
       CLCD_Text(screen.front,5,405,WHITE,desc); 
    
    
    //Load the AVI file from memory and parse
#if AVI_FILE == TINY
    movie.avifile = AVI_open_input_file(video_start0,video_end0,1);
#elif AVI_FILE == SMALL
    movie.avifile = AVI_open_input_file(video_start1,video_end1,1);
#elif AVI_FILE == MEDIUM    
    movie.avifile = AVI_open_input_file(video_start2,video_end2,1);
#else
    movie.avifile = AVI_open_input_file(video_start3,video_end3,1);
#endif
        
    //Init xvid Decoder
    status = dec_init(movie.avifile);
    if (status)
    { 
        printf("\n  Decore INIT problem, return value %d\n", status);
        return -1;      
    }
  
    
do  {
    
    
    bytes_processed = 0; // Bytes processed in working_buffer
    



    // Start Timer
    CNT_Start();  
      
    // Refill working buffer if almost empty
    if (bytes_buffered <10) {
        bytes_buffered = AVI_read_frame(movie.avifile,working_buffer,(int *)&key_frame); 
        mp4_ptr = (unsigned char*)working_buffer;
    }
    
    // Load ticks
    load_ticks = CNT_End(COUNTER_CCNT);

    // Set frame pointer to non-active screen memory (double buffering)
    video_pointer = (uint16_t *)screen.back;
    
    // Start Timer
    CNT_Start();

    // Call to xvid decoder
    bytes_processed = dec_main(movie.avifile, mp4_ptr, (unsigned char *) video_pointer, bytes_buffered, &xvid_dec_stats);
    

    // Decode ticks
    dec_ticks = CNT_End(COUNTER_CCNT);
        

    
    
    // Update working buffer position
    if (bytes_processed > 0) 
    {  
        mp4_ptr += bytes_processed;
        bytes_buffered -= bytes_processed;
    }
    
    
    // Check for overflow of counters
    if( (dec_ticks==0xFFFFFFFF) || (load_ticks==0xFFFFFFFF)) while(1);


    // PRACTICAL 9.1.3
    // ---------------
    
    // TODO: Comment out the dec_ticks and load_ticks accumulate lines below    
    
    
    // TODO: Move the two unsigned integer values (dec_ticks and 
    // load_ticks) into a NEON vector
    
    // Note: Use the following neon intrinsic:
    // vset_lane_u32() - Move a 32-bit unsigned int into a lane
    
    neon_ticks = vset_lane_u32(dec_ticks,neon_ticks,1);
    neon_ticks = vset_lane_u32(load_ticks,neon_ticks,0);
    
    // ---------------
    
    // PRACTICAL 9.1.4
    // ---------------
    
    // TODO: Convert the input vector elements from 32 to 64 and
    // add the converted input vector to the output vector.
    
    // Note: Use the following neon intrinsics:
    // vmovl_u32() - Expand element size from 32 to 64
    // vaddq_u64() - Add 64-bit elements
    
    neon_totals = vaddq_u64(neon_totals, vmovl_u32(neon_ticks));

    // ---------------

    
            
    // If complete frame has been decoded, flip screen
    if (xvid_dec_stats.type > 0)
    {         
        frames++;
                    
        CLCD_Swap();
    }
    
    } while (FRAMES_PLAYBACK>frames);


    if(counters==COUNTERS_OK)
    {
       // PRACTICAL 9.1.5
           // ---------------
        
        // TODO: Extract each of the two unsigned integer elements in the NEON vector 
        // and move them into totdec_ticks and totload_ticks

        // Note: Use the following neon intrinsic:
        // vget_lane_u64() - Extract a 64-bit unsigned int (value returned by the intrinsic)
        
        totdec_ticks = vgetq_lane_u64(neon_totals,1);
        totload_ticks = vgetq_lane_u64(neon_totals,0);

        // ---------------


        avgdec_ticks = (double)totdec_ticks/(double)frames;
        avgload_ticks = (double)totload_ticks/(double)frames;

        


 
        sprintf(desc,"XVID benchmark:\n Platform: %s\n Architecture: %s\nTotal Cycles: %lld\nCycles per frame decode: %5.3lf (Cycles per frame load: %5.3lf)\n",platform_desc,architecture_desc,totdec_ticks+totload_ticks,avgdec_ticks,avgload_ticks);
        
#if(SEMIHOST)
        semiPrint(desc);
#endif    

        // Stats area final
        CLCD_Block(screen.front,0,400,640,80,BLACK);
        CLCD_Rect(screen.front,0,400,640,80,WHITE);
        CLCD_Text(screen.front,5,405,WHITE,desc);
        
        CLCD_Block(screen.back,0,400,640,80,BLACK);
        CLCD_Rect(screen.back,0,400,640,80,WHITE);
        CLCD_Text(screen.back,5,405,WHITE,desc);
    }
    
    return 0;

}
