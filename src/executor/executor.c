/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jimin <jimin@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/08/29 18:15:22 by jimin             #+#    #+#             */
/*   Updated: 2022/08/30 23:19:00 by jimin            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	execute_command(t_process *process)
{
	char	*command;
	char	**argv_curr;
	char	**paths;

	command = process->argv[0];
	paths = g_minishell_info.ps_list->paths;
	command = get_accessable_command(command, paths);
	if (command)
	{
		if (execve(command, process->argv, process->envp))
			error_two_exit_status(127, command, "command not found");
	}
	else
		error_two_exit_status(127, "", "command not found");
}

void	execute_process(t_process *process, t_pipes *pipes)
{
	safe_dup2(pipes->prev_pipe[READ], STDIN_FILENO);
	safe_dup2(pipes->next_pipe[WRITE], STDOUT_FILENO);
	safe_close_pipes(pipes);
	if (apply_redirections(process->cmd_line) == FAILURE)
		exit(EXIT_FAILURE);
	if (is_argv_null(process->argv))
		exit(EXIT_SUCCESS);
	if (is_built_in(process))
		exit(execute_built_in(process));
	else
		execute_command(process);
}

int	execute_pipeline(void)
{
	int			idx;
	t_process	*ps_curr;

	idx = 0;
	ps_curr = g_minishell_info.ps_list;
	init_pipe(&g_minishell_info.pipes);
	signal(SIGINT, kill_all_childs);
	while (ps_curr)
	{
		swap_pipe(&g_minishell_info.pipes);
		if (ps_curr->next != 0 && pipe(g_minishell_info.pipes.next_pipe) == -1)
			error_exit("fail_pipe()");
		ps_curr->pid = fork();
		if (ps_curr->pid == -1)
			error_exit("fail fork()\n");
		else if (ps_curr->pid == 0)
			execute_process(ps_curr, &(g_minishell_info.pipes));
		else
		{
			safe_close_pipe(&g_minishell_info.pipes.prev_pipe[READ]);
			safe_close_pipe(&g_minishell_info.pipes.next_pipe[WRITE]);
		}
		ps_curr = ps_curr->next;
	}
	return (wait_childs());
}

void	execute_single_cmdline(void)
{
	t_process	*process;

	process = g_minishell_info.ps_list;
	signal(SIGINT, kill_all_childs);
	if (apply_redirections(process->cmd_line) == FAILURE)
	{
		set_last_status(EXIT_FAILURE);
		return ;
	}
	if (is_argv_null(process->argv))
		return ;
	if (is_built_in(process))
	{
		set_last_status(execute_built_in(process));
		return ;
	}
	g_minishell_info.ps_list->pid = fork();
	if (g_minishell_info.ps_list->pid == -1)
		error_exit("fork error");
	else if (g_minishell_info.ps_list->pid == 0)
		execute_command(process);
	else
		set_last_status(wait_child(g_minishell_info.ps_list->pid));
	return ;
}

void	executor(void)
{
	int	status;

	if (is_heredoc_error())
	{
		print_error_message("too many heredoc");
		return ;
	}
	if (execute_heredoc())
		return ;
	if (g_minishell_info.ps_list->size == 1)
		execute_single_cmdline();
	else
		set_last_status(execute_pipeline());
	signal(SIGINT, get_new_prompt);
	restore_stdio();
}
