#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n ->parent = NULL;
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}


/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){

    node_t* new_node = NULL;
	new_node= (node_t *) malloc(sizeof(node_t));
	new_node->parent = n;
	new_node->depth = (n -> depth) + 1 ;
	new_node->move = action;
	copy_state(&(new_node->state),&(n->state));
	new_node->state.cursor = *selected_peg;
    execute_move_t(&(new_node->state), &(new_node->state.cursor), action);
	
	return new_node;

}

/**
 * Find a solution path as per algorithm description in the handout
 */
 
void find_solution(state_t* init_state){
	int remaining_peck;
	HashTable table;
	
	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);
	
	// Initialize Stack
	initialize_stack();
	
	//Add the initial node 
	node_t* n = create_init_node( init_state );
	
	//create stack array
	node_t* new_node, **stack_array=malloc((budget+1)*sizeof(node_t));
	
	move_t side;
	position_s peck_position;
	stack_push(n);
	remaining_peck = num_pegs(init_state);
	
	while (! is_stack_empty()){
		n = stack_top(); 
		stack_pop();
		stack_array[expanded_nodes]=n;
		expanded_nodes++;
		
		if (num_pegs(&(n -> state)) < remaining_peck){
			save_solution(n);
			remaining_peck = num_pegs(&(n -> state));
		}
		
		//for loop to check if they are able to do a jump 
		for (int i = 0; i < SIZE; i++){
			for (int j = 0; j < SIZE ;j++){
				for ( int jump = 0; jump < 4;jump++){
					peck_position.x = i;
					peck_position.y = j;
					
					 // if the jump is legal
					if(can_apply(&(n -> state), &peck_position, jump)){
						new_node = applyAction( n, &peck_position, jump);
						generated_nodes=generated_nodes+1;
						
						// if only a peck was left
						if (won(&(new_node -> state))){
							save_solution(new_node);
							remaining_peck = num_pegs(&(new_node -> state));
							
							//free node, linkedlist of table and array of stack
							for (int h=0; h<expanded_nodes;h++){
								free(stack_array[h]);
							}
							free(stack_array);
							free(new_node);
							ht_clear(&table);
							ht_destroy( &table);
							free_stack();
							return;
						}
						// if seen for the first time
						if ( !ht_contains(&table, new_node -> state.field)){
							ht_insert(&table, new_node -> state.field, new_node -> state.field); 
							stack_push(new_node);
						
						}else{
							free(new_node);
						}
					}	
				}
			}		
		}
		//if budget exhausted
		if (expanded_nodes >= budget){		
			break ;
		}
	}
	
	// free everything
	ht_clear(&table);
	ht_destroy( &table);
	free_stack();
	for (int h=0; h<expanded_nodes;h++){
		free(stack_array[h]);
	}
	free(stack_array);
}
