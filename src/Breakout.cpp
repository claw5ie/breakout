#include "Utils.hpp"
#include "Breakout.hpp"

Breakout
create_breakout (void)
{
  static_assert (sizeof (AABB) == 4 * sizeof (float), ":(");

  Breakout game;

  game.block_count = 6 * 5;
  game.blocks = (AABB *)malloc_or_exit (game.block_count * sizeof (AABB));

  game.slab = { { 0.5, -0.8 }, { 0.3, 0.016 } };
  game.slab_vel = { 0.03, 0.0 };

  game.ball = { { 0.0, -0.8 }, { 0.05, 0.05 } };
  game.ball_vel = { 0.01, 0.01 };

  glCreateVertexArrays (1, &game.vertex_array);
  glCreateBuffers (2, (gluint *)game.buffer);

  {
    auto vertex_shader =
      create_shader (GL_VERTEX_SHADER, "shaders/quad.vert");
    auto fragment_shader =
      create_shader (GL_FRAGMENT_SHADER, "shaders/quad.frag");
    game.program =
      create_program (vertex_shader, fragment_shader);

    glDeleteShader (vertex_shader);
    glDeleteShader (fragment_shader);
  }

  glBindVertexArray (game.vertex_array);

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[0]);
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (0);

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
  glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (1);
  glVertexAttribDivisor (1, 1);

  {
    Vec2f quad[4] = { { 0.0, 0.0 },
                      { 1.0, 0.0 },
                      { 0.0, 1.0 },
                      { 1.0, 1.0 } };

    glBindBuffer (GL_ARRAY_BUFFER, game.buffer[0]);
    glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
  }

  {
    float x_offset = 0.04, y_offset = 0.04;
    AABB block =
      { { -1 + x_offset, 1 - y_offset },
        { (2 - (6 + 1) * x_offset) / 6, 0.05 } };

    block.pos.y -= block.shape.y;

    for (size_t i = 0; i < game.block_count; )
      {
        game.blocks[i] = block;

        if (++i % 6 != 0)
          block.pos.x += x_offset + block.shape.x;
        else
          {
            block.pos.x = -1 + x_offset;
            block.pos.y -= y_offset + block.shape.y;
          }
      }
  }

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
  glBufferData (GL_ARRAY_BUFFER,
                (game.block_count + 2) * sizeof (AABB),
                NULL,
                GL_DYNAMIC_DRAW);

  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::slab_offset,
                   sizeof (AABB),
                   &game.slab);

  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::ball_offset,
                   sizeof (AABB),
                   &game.ball);

  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::block_offset,
                   game.block_count * sizeof (AABB),
                   game.blocks);

  return game;
}

void
resolve_collisions (Breakout &game)
{
  for (size_t i = 0; i < game.block_count; i++)
    {
      AABB &block = game.blocks[i];

      if (do_intersect (game.ball, block))
        {
          switch (hit_direction (block, game.ball, game.ball_vel))
            {
            case Left:
            case Right:
              game.ball_vel.x = -game.ball_vel.x;
              break;
            case Down:
            case Up:
              game.ball_vel.y = -game.ball_vel.y;
              break;
            }

          block.pos = { -2, -2 };

          glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
          glBufferSubData (GL_ARRAY_BUFFER,
                           Breakout::block_offset + i * sizeof (AABB),
                           sizeof (AABB),
                           &game.blocks[i]);

          break;
        }
    }

  if (do_intersect (game.ball, game.slab))
    {
      switch (hit_direction (game.slab, game.ball, game.ball_vel))
        {
        case Left:
        case Right:
          game.ball_vel.x = -game.ball_vel.x;
          break;
        case Down:
        case Up:
          game.ball_vel.y = -game.ball_vel.y;
          break;
        }
    }
}

void
update (Breakout &game)
{
  if (game.ball.pos.x <= -1
      || game.ball.pos.x + game.ball.shape.x >= 1)
    game.ball_vel.x = -game.ball_vel.x;
  else if (game.ball.pos.y <= -1
           || game.ball.pos.y + game.ball.shape.y >= 1)
    game.ball_vel.y = -game.ball_vel.y;

  resolve_collisions (game);

  game.ball.pos += game.ball_vel;

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::ball_offset,
                   sizeof (AABB),
                   &game.ball);
}

void
draw (const Breakout &game)
{
  glBindVertexArray (game.vertex_array);
  glUseProgram (game.program);
  glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, 2 + game.block_count);
}
