/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU Go, a Go program. Contact gnugo@gnu.org, or see       *
 * http://www.gnu.org/software/gnugo/ for more information.          *
 *                                                                   *
 * Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,   *
 * 2008 and 2009 by the Free Software Foundation.                    *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License as    *
 * published by the Free Software Foundation - version 3 or          *
 * (at your option) any later version.                               *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License in file COPYING for more details.      *
 *                                                                   *
 * You should have received a copy of the GNU General Public         *
 * License along with this program; if not, write to the Free        *
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,       *
 * Boston, MA 02111, USA.                                            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gnugo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interface.h"
#include "sgftree.h"
#include "gg_utils.h"
#include "liberty.h"

static void replay_node(SGFNode *node, int color_to_test, float *replay_score,
                        float *total_score);


/* --------------------------------------------------------------*/
/* replay a game */
/* --------------------------------------------------------------*/

void
play_replay(SGFTree *tree, int color_to_replay)
{
  char *tmpc = NULL;
  float replay_score = 0.0;
  float total_score = 0.0;

  SGFNode *node = tree->root;

  /* Board size and komi are already set up correctly since the game
   * has already been loaded before this function is called. Now we
   * only have to clear the board before starting over.
   */
  clear_board();

  if (!quiet) {
    printf("Board Size:   %d\n", board_size);
    if (sgfGetCharProperty(node, "HA", &tmpc))
      printf("Handicap:     %s\n", tmpc);
    printf("Komi:         %.1f\n", komi);
    if (sgfGetCharProperty(node, "RU", &tmpc))
      printf("Ruleset:      %s\n", tmpc);
    if (sgfGetCharProperty(node, "GN", &tmpc))
      printf("Game Name:    %s\n", tmpc);
    if (sgfGetCharProperty(node, "DT", &tmpc))
      printf("Game Date:    %s\n", tmpc);
    if (sgfGetCharProperty(node, "GC", &tmpc))
      printf("Game Comment: %s\n", tmpc);
    if (sgfGetCharProperty(node, "US", &tmpc))
      printf("Game User:    %s\n", tmpc);
    if (sgfGetCharProperty(node, "PB", &tmpc))
      printf("Black Player: %s\n", tmpc);
    if (sgfGetCharProperty(node, "PW", &tmpc))
      printf("White Player: %s\n", tmpc);
    if (sgfGetCharProperty(node, "RE", &tmpc))
      printf("Result:       %s\n", tmpc);
  }

  /*
   * Now actually run through the file.  This is the interesting part.
   * We need to traverse the SGF tree, and every time we encounter a node
   * we need to check what move GNU Go would make, and see if it is OK.
   */
  while (node) {
    replay_node(node, color_to_replay, &replay_score, &total_score);
    sgffile_output(tree);
    node = node->child;
  }

  if (!quiet)
    printf("Global score: %.2f / %.2f\n", replay_score, total_score);

  if (showtime) {
    gprintf("SLOWEST MOVE: %d at %1m ", slowest_movenum, slowest_move);
    fprintf(stderr, "(%.2f seconds)\n", slowest_time);
    fprintf(stderr, "AVERAGE TIME: %.2f seconds per move\n",
            total_time / movenum);
    fprintf(stderr, "TOTAL TIME: %.2f seconds\n",
            total_time);
  }
}


#define BUFSIZE 128

/*
 * Handle this node.
 */

static void
replay_node(SGFNode *node, int color_to_replay, float *replay_score,
            float *total_score)
{
  SGFProperty *sgf_prop;  /* iterate over properties of the node */
  SGFProperty *move_prop = NULL; /* remember if we see a move property */
  int color; /* color of move to be made at this node. */

  int old_move; /* The move played in the file. */
  int new_move; /* The move generated by GNU Go. */

  char buf[BUFSIZE];

  /* Handle any AB / AW properties, and note presence
   * of move properties.
   */

  for (sgf_prop = node->props; sgf_prop; sgf_prop = sgf_prop->next) {
    switch (sgf_prop->name) {
    case SGFAB:
      /* add black */
      add_stone(get_sgfmove(sgf_prop), BLACK);
      break;
    case SGFAW:
      /* add white */
      add_stone(get_sgfmove(sgf_prop), WHITE);
      break;
    case SGFB:
    case SGFW:
      move_prop = sgf_prop;  /* remember it for later */
      break;
    }
  }

  /* Only generate moves at move nodes. */
  if (!move_prop)
    return;

  old_move = get_sgfmove(move_prop);
  color = (move_prop->name == SGFW) ? WHITE : BLACK;

  if (color == color_to_replay || color_to_replay == GRAY) {
    float new_move_value = 0.0;
    float old_move_value = 0.0;

    /* Get a move from the engine for color. */
    int resign;
    new_move = genmove(color, NULL, &resign);

    /* Pick up the relevant values from the potential_moves[] array. */
    if (new_move != PASS_MOVE)
      new_move_value = potential_moves[new_move];
    if (old_move != PASS_MOVE)
      old_move_value = potential_moves[old_move];

    /* Now report on how well the computer generated the move. */
    if (new_move != old_move || !quiet) {
      mprintf("Move %d (%C): ", movenum + 1, color);

      if (resign)
        printf("GNU Go resigns ");
      else {
        mprintf("GNU Go plays %1m ", new_move);
        if (new_move != PASS_MOVE)
          printf("(%.2f) ", new_move_value);
      }

      mprintf("- Game move %1m ", old_move);
      if (new_move != PASS_MOVE && old_move_value > 0.0)
        printf("(%.2f) ", old_move_value);
      printf("\n");

      *replay_score += new_move_value - old_move_value;
      *total_score += new_move_value;
    }

    if (new_move != old_move) {
      if (resign)
        gg_snprintf(buf, BUFSIZE, "GNU Go resigns - Game move %s (%.2f)",
                    location_to_string(old_move), old_move_value);
      else {
        gg_snprintf(buf, BUFSIZE,
                    "GNU Go plays %s (%.2f) - Game move %s (%.2f)",
                    location_to_string(new_move), new_move_value,
                    location_to_string(old_move), old_move_value);
        if (new_move != PASS_MOVE)
          sgfCircle(node, I(new_move), J(new_move));
      }
    }
    else
      gg_snprintf(buf, BUFSIZE, "GNU Go plays the same move %s (%.2f)",
                  location_to_string(new_move), new_move_value);

    sgfAddComment(node, buf);
    sgffile_add_debuginfo(node, 0.0);
  }

  /* Finally, do play the move from the file. */
  play_move(old_move, color);
}


/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
