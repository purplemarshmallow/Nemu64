#ifndef S2DEX_HEADER_BLAINCLUDED
#define S2DEX_HEADER_BLAINCLUDED

typedef	struct	{
  U16	imageW;		/* The width of the texture.                                    (u10.2)*/
  U16   imageX;		/* The x-coordinate of the upper-left position of the texture.  (u11.5)*/ 
  U16	frameW;		/* The width of the transferred frame.                          (u10.2)*/
  S16	frameX;		/* The upper-left position of the transferred frame.            (s10.2)*/

  U16	imageH;		/* The height of the texture.                                   (u10.2)*/
  U16   imageY; 	/* The y-coordinate of the upper-left position of the texture.  (u11.5)*/ 
  U16	frameH;		/* The height of the transferred frame.                         (u10.2)*/
  S16	frameY;		/* The upper-left position of the transferred frame.            (s10.2)*/

  // changed from U64*
  U32   imagePtr;	/* The texture source address on DRAM.           */
  U8	imageSiz;	/* The size of the texel        G_IM_SIZ_*       */
  U8	imageFmt;	/* The format of the texel.  G_IM_FMT_*          */
  U16	imageLoad;	/* Which to use,  LoadBlock or  LoadTile?        */
  U16	imageFlip;	/* The right & left inversion of the image. Inverted by G_BG_FLAG_FLIPS*/
  U16   imagePal; 	/* The pallet number.                            */

  /* Because the following are set in the initialization routine guS2DInitBg(), the user doesn't 
     have to set it.*/
  U16	tmemH;		/* The height of TMEM loadable at a time.  (s13.2) The 4 times value.
			   When the normal texture,   512/tmemW*4
			   When the CI texture,    256/tmemW*4       */
  U16	tmemW;		/* The TMEM width and Work size of the frame 1 line. 
                           At LoadBlock, GS_PIX2TMEM(imageW/4,imageSiz)
                           At LoadTile  GS_PIX2TMEM(frameW/4,imageSiz)+1  */
  U16	tmemLoadTH;	/* The TH value or the Stride value 
			   At LoadBlock,  GS_CALC_DXT(tmemW)
			   At LoadTile,  tmemH-1                     */
  U16	tmemLoadSH;	/* The SH value
			   At LoadBlock,  tmemSize/2-1
			   At LoadTile,  tmemW*16-1                  */
  U16	tmemSize;	/* The skip value of  imagePtr for 1-loading.  
			   = tmemSizeW*tmemH                         */
  U16	tmemSizeW;	/* The skip value of imagePtr for image 1-line.  
			   At LoadBlock, tmemW*2
                           At LoadTile,  GS_PIX2TMEM(imageW/4,imageSiz)*2 */
} uObjBg_t;		/* 40 bytes */

/* Scalable background plane */
typedef	struct	{
  U16	imageW;		/* The width of the texture.                                    (u10.2)*/
  U16   imageX;		/* The x-coordinate of the upper-left position of the texture.  (u11.5)*/ 
  U16	frameW;		/* The width of the transferred frame.                          (u10.2)*/
  S16	frameX;		/* The upper-left position of the transferred frame.            (s10.2)*/

  U16	imageH;		/* The height of the texture.                                   (u10.2)*/
  U16   imageY; 	/* The y-coordinate of the upper-left position of the texture.  (u11.5)*/ 
  U16	frameH;		/* The height of the transferred frame.                         (u10.2)*/
  S16	frameY;		/* The upper-left position of the transferred frame.            (s10.2)*/

  // changed from U64*
  U32   imagePtr;	/* The texture source address on DRAM.       */
  U8	imageSiz;	/* The size of the texel      G_IM_SIZ_*     */
  U8	imageFmt;	/* The format of the texel.   G_IM_FMT_*     */
  U16	imageLoad;	/* Which to use,  LoadBlock or  LoadTile?    */
  U16	imageFlip;	/* The right & left inversion of the image. Inverted by G_BG_FLAG_FLIPS*/
  U16   imagePal; 	/* The pallet number.                        */

  U16	scaleH;		/* The scale value of the Y-direction.           (u5.10)*/
  U16	scaleW;		/* The scale value of the X-direction.           (u5.10)*/
  S32	imageYorig;	/* The start point of drawing on the image.      (s20.5)*/
  
  U32	padding;
  
} uObjScaleBg_t;	/* 40 bytes */

typedef union {
  uObjBg_t        b;
  uObjScaleBg_t   s;
} uObjBg;


// TEXTURE LOADING AND DRAWING SPRITES

typedef	struct	{
  U32	type;		/* G_OBJLT_TXTRBLOCK divided into types.                                */
  U32	image;		/* The texture source address on DRAM.                                  */
  U16	tsize;		/* The Texture size.  Specified by the macro  GS_TB_TSIZE().            */
  U16	tmem;		/* The  transferred TMEM word address.   (8byteWORD)                    */
  U16	sid;		/* STATE ID Multipled by 4.  Either one of  0,4,8 and 12.               */
  U16	tline;		/* The width of the Texture 1-line. Specified by the macro GS_TB_TLINE()*/
  U32	flag;		/* STATE flag  */
  U32	mask;		/* STATE mask  */
} uObjTxtrBlock_t;		/* 24 bytes */

#define	GS_TT_TWIDTH(pix,siz)	((GS_PIX2TMEM((pix), (siz))<<2)-1)
#define	GS_TT_THEIGHT(pix,siz)	(((pix)<<2)-1)

typedef	struct	{
  U32	type;		/* G_OBJLT_TXTRTILE divided into types.                             */
  U32	image;		/* The texture source address on DRAM.                              */
  U16	twidth;		/* The width of the Texture. Specified by the macro GS_TT_TWIDTH()  */
  U16	tmem;		/* The loaded texture source address on DRAM.  (8byteWORD)          */
  U16	sid;		/* STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.          */
  U16	theight;	/* The height of the Texture. Specified by the macro GS_TT_THEIGHT()*/
  U32	flag;		/* STATE flag  */
  U32	mask;		/* STATE mask  */

} uObjTxtrTile_t;		/* 24 bytes */

#define	GS_PAL_HEAD(head)	((head)+256)
#define	GS_PAL_NUM(num)		((num)-1)

typedef	struct	{
  U32	type;		/* G_OBJLT_TLUT divided into types.                            */
  U32	image;		/* the texture source address on DRAM.                         */
  U16	pnum;		/* The loading pallet number -1.                               */
  U16	phead;		/* The pallet number of the load header.  Between 256 and 511. */
  U16	sid;		/* STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.     */
  U16   zero;		/* Assign 0 all the time.                                      */
  U32	flag;		/* STATE flag  */
  U32	mask;		/* STATE mask  */
} uObjTxtrTLUT_t;		/* 24 bytes */

typedef union {
  uObjTxtrBlock_t      block;
  uObjTxtrTile_t       tile;
  uObjTxtrTLUT_t       tlut;
  __int64   force_structure_alignment;
} uObjTxtr;


typedef struct {
  U16  scaleW;		/* Scaling of the u5.10 width direction.                            */
  S16  objX;		/* The x-coordinate of the upper-left end. s10.2 OBJ                */
  U16  paddingX;	/* Unused.  Always 0.                                               */
  U16  imageW;		/* The width of the u10.5 texture. (The length of the S-direction.) */
  U16  scaleH;		/* Scaling of the u5.10 height direction.                           */
  S16  objY;		/* The y-coordinate of the s10.2 OBJ upper-left end.                */
  U16  paddingY;	/* Unused.  Always 0.                                               */
  U16  imageH;		/* The height of the u10.5 texture. (The length of the T-direction.)*/
  U16  imageAdrs;	/* The texture header position in  TMEM.  (In units of 64bit word.) */  
  U16  imageStride;	/* The folding width of the texel.        (In units of 64bit word.) */
  U8   imageFlags;	/* The display flag.    G_OBJ_FLAG_FLIP*            */
  U8   imagePal;	/* The pallet number.  0-7                        */
  U8   imageSiz;	/* The size of the texel.         G_IM_SIZ_*       */
  U8   imageFmt;	/* The format of the texel.   G_IM_FMT_*       */
} uObjSprite_t;		/* 24 bytes */

typedef union {
  uObjSprite_t      s;
} uObjSprite;


typedef	struct	{
  uObjTxtr	txtr;
  uObjSprite	sprite;
} uObjTxSprite;		/* 48 bytes */

typedef struct {
  S32   A, B, C, D;     /* s15.16 */
  S16   Y, X;           /* s10.2  */
  S16   BaseScaleY;	/* u5.10  */
  S16   BaseScaleX;	/* u5.10  */
} uObjMtx_t;		/* 24 bytes */

typedef union {
  uObjMtx_t	m;
  __int64 force_structure_alignment;
} uObjMtx;

typedef struct {
  S16   Y, X;           /* s10.2  */
  U16   BaseScaleY;	/* u5.10  */
  U16   BaseScaleX;	/* u5.10  */
} uObjSubMtx_t;		/* 8 bytes */

typedef union {
  uObjSubMtx_t	m;
  __int64 force_structure_alignment;
} uObjSubMtx;

#define	G_OBJLT_TXTRBLOCK	0x00001033
#define	G_OBJLT_TXTRTILE	0x00fc1034
#define	G_OBJLT_TLUT		0x00000030

#endif