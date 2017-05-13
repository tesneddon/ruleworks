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

typedef union  {
  Ast_Node	node;		/* Nonterminals */
  Pred_Type	pred;		/* For predicate nonterminal */
  long		ival;
  Token_Value	tok;		/* Terminals (tokens) */
} YYSTYPE;
extern YYSTYPE yylval;
# define TOK_INTEGER_CONST 257
# define TOK_FLOAT_CONST 258
# define TOK_VARIABLE 259
# define TOK_QUOTED_VAR 260
# define TOK_SYMBOL_CONST 261
# define TOK_QUOTED_SYMBOL 262
# define TOK_OPAQUE_CONST 263
# define TOK_INSTANCE_CONST 264
# define TOK_LPAREN 265
# define TOK_RPAREN 266
# define TOK_LBRACE 267
# define TOK_RBRACE 268
# define TOK_LBRACKET 269
# define TOK_RBRACKET 270
# define TOK_HAT 271
# define TOK_OR 272
# define TOK_AND 273
# define TOK_NOT 274
# define TOK_PLUS 275
# define TOK_MINUS 276
# define TOK_TIMES 277
# define TOK_DIVIDE 278
# define TOK_MODULUS 279
# define UMINUS 280
# define TOK_START_DISJUNCTION 281
# define TOK_END_DISJUNCTION 282
# define TOK_EQUAL 283
# define TOK_EQUAL_EQUAL 284
# define TOK_APPROX_EQUAL 285
# define TOK_NOT_APPROX_EQUAL 286
# define TOK_SAME_TYPE 287
# define TOK_DIFF_TYPE 288
# define TOK_NOT_EQ 289
# define TOK_NOT_EQUAL 290
# define TOK_LESS 291
# define TOK_LESS_EQUAL 292
# define TOK_GREATER 293
# define TOK_GREATER_EQUAL 294
# define TOK_CONTAINS 295
# define TOK_DOES_NOT_CONTAIN 296
# define TOK_LENGTH_LESS_EQUAL 297
# define TOK_LENGTH_NOT_EQUAL 298
# define TOK_LENGTH_LESS 299
# define TOK_LENGTH_EQUAL 300
# define TOK_LENGTH_GREATER_EQUAL 301
# define TOK_LENGTH_GREATER 302
# define TOK_ACCEPT 303
# define TOK_ACCEPT_ATOM 304
# define TOK_ACCEPTLINE_COMPOUND 305
# define TOK_ACCEPTS 306
# define TOK_ACTIVATES 307
# define TOK_ADDSTATE 308
# define TOK_AFTER 309
# define TOK_ALIAS_FOR 310
# define TOK_ANY 311
# define TOK_APPEND 312
# define TOK_ARROW 313
# define TOK_ASCID 314
# define TOK_ASCIZ 315
# define TOK_AT 316
# define TOK_ATOM 317
# define TOK_BIND 318
# define TOK_BY 319
# define TOK_BYTE 320
# define TOK_CALL_INHERITED 321
# define TOK_CATCH 322
# define TOK_CLOSEFILE 323
# define TOK_COMPOUND 324
# define TOK_CONCAT 325
# define TOK_COPY 326
# define TOK_CRLF 327
# define TOK_CS 328
# define TOK_DEBUG 329
# define TOK_DECLARATION_BLOCK 330
# define TOK_DEFAULT 331
# define TOK_DO 332
# define TOK_DOLLAR_LAST 333
# define TOK_DOUBLE_FLOAT 334
# define TOK_EB 335
# define TOK_ELSE 336
# define TOK_END_BLOCK 337
# define TOK_END_GROUP 338
# define TOK_ENTRY_BLOCK 339
# define TOK_EVERY 340
# define TOK_EXTERNAL_ROUTINE 341
# define TOK_FAILURE 342
# define TOK_FILENAME 343
# define TOK_FILL 344
# define TOK_FLOAT 345
# define TOK_FOR_EACH 346
# define TOK_GENATOM 347
# define TOK_GENERIC_METHOD 348
# define TOK_GENINT 349
# define TOK_GET 350
# define TOK_IF 351
# define TOK_IN 352
# define TOK_INHERITS_FROM 353
# define TOK_INSTANCE 354
# define TOK_INTEGER 355
# define TOK_IS_OPEN 356
# define TOK_LENGTH 357
# define TOK_LEX 358
# define TOK_LONG 359
# define TOK_MAKE 360
# define TOK_MAX 361
# define TOK_MEA 362
# define TOK_METHOD 363
# define TOK_MIN 364
# define TOK_MODIFY 365
# define TOK_NTH 366
# define TOK_OBJECT_CLASS 367
# define TOK_OF 368
# define TOK_OFF 369
# define TOK_ON 370
# define TOK_ON_EMPTY 371
# define TOK_ON_ENTRY 372
# define TOK_ON_EVERY 373
# define TOK_ON_EXIT 374
# define TOK_OPAQUE 375
# define TOK_OPENFILE 376
# define TOK_OUT 377
# define TOK_P 378
# define TOK_PATHNAME 379
# define TOK_POINTER 380
# define TOK_POSITION 381
# define TOK_QUIT 382
# define TOK_READ_ONLY 383
# define TOK_READ_WRITE 384
# define TOK_REFERENCE 385
# define TOK_REMOVE_EVERY 386
# define TOK_REMOVE 387
# define TOK_RESTORESTATE 388
# define TOK_RETURN 389
# define TOK_RETURNS 390
# define TOK_RG 391
# define TOK_RJUST 392
# define TOK_RULE 393
# define TOK_RULE_BLOCK 394
# define TOK_RULE_GROUP 395
# define TOK_SAVESTATE 396
# define TOK_SHORT 397
# define TOK_SINGLE_FLOAT 398
# define TOK_SCALAR 399
# define TOK_SPECIALIZE 400
# define TOK_SQL_ATTACH 401
# define TOK_SQL_COMMIT 402
# define TOK_SQL_DELETE 403
# define TOK_SQL_DETACH 404
# define TOK_SQL_FETCH_AS_OBJECT 405
# define TOK_SQL_FETCH_EACH 406
# define TOK_SQL_INSERT 407
# define TOK_SQL_INSERT_FROM_OBJECT 408
# define TOK_SQL_ROLLBACK 409
# define TOK_SQL_START 410
# define TOK_SQL_UPDATE 411
# define TOK_SQL_UPDATE_FROM_OBJECT 412
# define TOK_STARTUP 413
# define TOK_STRATEGY 414
# define TOK_SUBCOMPOUND 415
# define TOK_SUBSYMBOL 416
# define TOK_SUCCESS 417
# define TOK_SYMBOL 418
# define TOK_SYMBOL_LENGTH 419
# define TOK_TABTO 420
# define TOK_THEN 421
# define TOK_TRACE 422
# define TOK_UNSIGNED_BYTE 423
# define TOK_UNSIGNED_LONG 424
# define TOK_UNSIGNED_SHORT 425
# define TOK_USES 426
# define TOK_VALUE 427
# define TOK_WHEN 428
# define TOK_WHILE 429
# define TOK_WM 430
# define TOK_WRITE 431
