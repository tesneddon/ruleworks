/*
RuleWorks - Rules based application development tool.

Copyright (C) 1999  Compaq Computer Corporation

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or any later 
version. 

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
Public License for more details. 

You should have received a copy of the GNU General Public License along 
with this program; if not, write to the Free Software Foundation, 
Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Email: info@ruleworks.co.uk

*/

typedef union  {	/* This union is where the parser values are stored:        */
  Token_Value    tok;	/* keeps track of constant token values - terminals */
  Molecule       mol;	/* this is where a molecularized constant is stored */
  Class          cls;	/* symbols are transformed into object-classes	    */
  Object         obj;
  void          *ptr;
  IO_Catagory    ioc;
  IO_Access_Mode ioa;
/*
  Pat_Pattern   *pat;
  Pat_Classes   *pc;
  Pat_Attrs     *pa;
  Pat_Tests     *pt;
*/
} YYSTYPE;
extern YYSTYPE yylval;
# define TOK_EOL 257
# define TOK_EOF 258
# define TOK_INTEGER_CONST 259
# define TOK_INSTANCE_CONST 260
# define TOK_OPAQUE_CONST 261
# define TOK_FLOAT_CONST 262
# define TOK_LPAREN 263
# define TOK_RPAREN 264
# define TOK_LBRACE 265
# define TOK_RBRACE 266
# define TOK_LBRACKET 267
# define TOK_RBRACKET 268
# define TOK_HAT 269
# define TOK_AMPERSAND 270
# define TOK_TILDE 271
# define TOK_START_DISJUNCTION 272
# define TOK_END_DISJUNCTION 273
# define TOK_PLUS 274
# define TOK_MINUS 275
# define TOK_TIMES 276
# define TOK_DIVIDE 277
# define TOK_MODULUS 278
# define UMINUS 279
# define TOK_ACCEPT 280
# define TOK_ACCEPT_ATOM 281
# define TOK_ACCEPTLINE_COMPOUND 282
# define TOK_ACCEPTS 283
# define TOK_ACTIVATES 284
# define TOK_ADDSTATE 285
# define TOK_AFTER 286
# define TOK_ALIAS_FOR 287
# define TOK_AND 288
# define TOK_ANY 289
# define TOK_APPEND 290
# define TOK_APPROX_EQUAL 291
# define TOK_ARROW 292
# define TOK_ASCID 293
# define TOK_ASCIZ 294
# define TOK_AT 295
# define TOK_ATOM 296
# define TOK_BIND 297
# define TOK_BLOCK_NAME 298
# define TOK_BY 299
# define TOK_BYTE 300
# define TOK_CATCH 301
# define TOK_CALL_INHERITED 302
# define TOK_CLOSEFILE 303
# define TOK_COMPOUND 304
# define TOK_CONTAINS 305
# define TOK_CONCAT 306
# define TOK_COPY 307
# define TOK_CRLF 308
# define TOK_CS 309
# define TOK_CTRLC 310
# define TOK_DECLARATION_BLOCK 311
# define TOK_DEBUG 312
# define TOK_DEFAULT 313
# define TOK_DIFF_TYPE 314
# define TOK_DISABLE 315
# define TOK_DO 316
# define TOK_DOES_NOT_CONTAIN 317
# define TOK_DOLLAR_LAST 318
# define TOK_DOUBLE_FLOAT 319
# define TOK_EBREAK 320
# define TOK_EB 321
# define TOK_ELSE 322
# define TOK_ENABLE 323
# define TOK_END_BLOCK 324
# define TOK_END_GROUP 325
# define TOK_ENTRY_BLOCK 326
# define TOK_EQUAL 327
# define TOK_EQUAL_EQUAL 328
# define TOK_EVERY 329
# define TOK_EXCISE 330
# define TOK_EXIT 331
# define TOK_EXTERNAL_ROUTINE 332
# define TOK_FAILURE 333
# define TOK_FILENAME 334
# define TOK_FILL 335
# define TOK_FLOAT 336
# define TOK_FOR_EACH 337
# define TOK_GENATOM 338
# define TOK_GENERIC_METHOD 339
# define TOK_GENINT 340
# define TOK_GET 341
# define TOK_GREATER 342
# define TOK_GREATER_EQUAL 343
# define TOK_IF 344
# define TOK_IN 345
# define TOK_INHERITS_FROM 346
# define TOK_INSTANCE 347
# define TOK_INTEGER 348
# define TOK_IS_OPEN 349
# define TOK_LENGTH 350
# define TOK_LENGTH_EQUAL 351
# define TOK_LENGTH_GREATER 352
# define TOK_LENGTH_GREATER_EQUAL 353
# define TOK_LENGTH_LESS 354
# define TOK_LENGTH_LESS_EQUAL 355
# define TOK_LENGTH_NOT_EQUAL 356
# define TOK_LESS 357
# define TOK_LESS_EQUAL 358
# define TOK_LEX 359
# define TOK_LONG 360
# define TOK_MAKE 361
# define TOK_MAX 362
# define TOK_MATCHES 363
# define TOK_MEA 364
# define TOK_METHOD 365
# define TOK_MIN 366
# define TOK_MODIFY 367
# define TOK_NEXT 368
# define TOK_NTH 369
# define TOK_NOT 370
# define TOK_NOT_APPROX_EQUAL 371
# define TOK_NOT_EQ 372
# define TOK_NOT_EQUAL 373
# define TOK_ON_EMPTY 374
# define TOK_ON_ENTRY 375
# define TOK_ON_EVERY 376
# define TOK_ON_EXIT 377
# define TOK_OBJECT_CLASS 378
# define TOK_OF 379
# define TOK_OFF 380
# define TOK_ON 381
# define TOK_OPAQUE 382
# define TOK_OPENFILE 383
# define TOK_OR 384
# define TOK_OUT 385
# define TOK_P 386
# define TOK_PATHNAME 387
# define TOK_RBREAK 388
# define TOK_POINTER 389
# define TOK_POSITION 390
# define TOK_PPCLASS 391
# define TOK_PPWM 392
# define TOK_PROMPT 393
# define TOK_READ_ONLY 394
# define TOK_READ_WRITE 395
# define TOK_QUIT 396
# define TOK_QUOTED_SYMBOL 397
# define TOK_QUOTED_VAR 398
# define TOK_RESTART 399
# define TOK_REFERENCE 400
# define TOK_REMOVE 401
# define TOK_REMOVE_EVERY 402
# define TOK_REPORT 403
# define TOK_RG 404
# define TOK_RUN 405
# define TOK_RESTORESTATE 406
# define TOK_RETURN 407
# define TOK_RETURNS 408
# define TOK_RJUST 409
# define TOK_RULE 410
# define TOK_RULE_BLOCK 411
# define TOK_RULE_GROUP 412
# define TOK_SAVESTATE 413
# define TOK_SAME_TYPE 414
# define TOK_SCALAR 415
# define TOK_SHORT 416
# define TOK_SHOW 417
# define TOK_SINGLE_FLOAT 418
# define TOK_SPACE 419
# define TOK_SPECIALIZE 420
# define TOK_SQL_ATTACH 421
# define TOK_SQL_COMMIT 422
# define TOK_SQL_DELETE 423
# define TOK_SQL_DETACH 424
# define TOK_SQL_FETCH_AS_OBJECT 425
# define TOK_SQL_FETCH_EACH 426
# define TOK_SQL_INSERT 427
# define TOK_SQL_INSERT_FROM_OBJECT 428
# define TOK_SQL_ROLLBACK 429
# define TOK_SQL_START 430
# define TOK_SQL_UPDATE 431
# define TOK_SQL_UPDATE_FROM_OBJECT 432
# define TOK_STARTUP 433
# define TOK_STRATEGY 434
# define TOK_SUBCOMPOUND 435
# define TOK_SUBSYMBOL 436
# define TOK_SUCCESS 437
# define TOK_SYMBOL 438
# define TOK_SYMBOL_CONST 439
# define TOK_SYMBOL_LENGTH 440
# define TOK_TABTO 441
# define TOK_THEN 442
# define TOK_TRACE 443
# define TOK_UNSIGNED_BYTE 444
# define TOK_UNSIGNED_LONG 445
# define TOK_UNSIGNED_SHORT 446
# define TOK_USES 447
# define TOK_VALUE 448
# define TOK_VARIABLE 449
# define TOK_VERSION 450
# define TOK_WARNING 451
# define TOK_WBREAK 452
# define TOK_WHEN 453
# define TOK_WHILE 454
# define TOK_WM 455
# define TOK_WMHISTORY 456
# define TOK_WRITE 457
