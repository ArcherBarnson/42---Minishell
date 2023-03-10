/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lcalvie <lcalvie@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/28 14:49:33 by bgrulois          #+#    #+#             */
/*   Updated: 2023/01/26 14:38:26 by mbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/minishell.h"

extern int	g_exit_code;

void	dup_fds(t_shell *shell)
{
	if (shell->cmd->fd_in > 0)
	{
		dup2(shell->cmd->fd_in, 0);
		close(shell->cmd->fd_in);
	}
	if (shell->cmd->fd_out > 1)
	{
		dup2(shell->cmd->fd_out, 1);
		close(shell->cmd->fd_out);
	}
	return ;
}

void	execute_command(t_shell *shell, char **envp, int mode)
{
	int	status;

	if (shell->cmd->fd_in != -1 && shell->cmd->fd_out != -1)
	{
		if (mode == 1)
		{
			status = exec_builtin(shell, 1);
			close_cmd_fds(shell->cmd);
			free(shell->cmd->cmd);
			ft_lstclear(&shell->cmd_head, del);
			shell->cmd = NULL;
			free_all(shell);
			exit(status);
		}
		dup_fds(shell);
		if (shell->cmd->cmd != NULL)
			execve(shell->cmd->cmd, shell->cmd->token, envp);
	}
	ft_free_hdoctab(shell->hdoc_tab);
	free(shell->cmd->cmd);
	shell->cmd->cmd = NULL;
	ft_lstclear(&shell->cmd_head, del);
	shell->cmd = NULL;
	free_all(shell);
}

int	simple_exec(t_shell *shell, char **envp)
{
	int	*pid;
	int	err_code;
	int	is_builtin;

	signal(SIGINT, SIG_IGN);
	is_builtin = check_builtins(shell);
	pid = make_pid_tab(cmds_get_n(shell));
	if (is_builtin == 1)
		return (free(pid), exec_builtin(shell, 0));
	err_code = check_for_invalid_cmd(shell);
	if (err_code > 1)
		return (free(pid), err_code);
	else if (err_code == 1)
		return (free(pid), 0);
	pid[0] = fork();
	if (pid[0] == 0)
	{
		free(pid);
		child_signals();
		execute_command(shell, envp, is_builtin);
		exit(1);
	}
	close_cmd_fds(shell->cmd);
	free_cmd_if(shell);
	return (ft_wait(pid, shell));
}

int	pipexec(t_shell *shell, int tbc, char **envp, int *pids)
{
	int	pid;
	int	is_builtin;
	int	err_code;

	err_code = 0;
	signal(SIGINT, SIG_IGN);
	is_builtin = check_builtins(shell);
	if (is_builtin == 2)
		exit(g_exit_code);
	if (is_builtin == 0)
	{
		err_code = check_for_invalid_cmd(shell);
		if (err_code)
			return (err_code);
	}
	pid = fork();
	if (pid == 0)
	{
		prep_pipexec(pids, tbc);
		execute_command(shell, envp, is_builtin);
		exit(1);
	}
	free_cmd_if(shell);
	return (pid);
}

int	pipeline(t_shell *shell, char **envp)
{
	int	i;
	int	*pids;

	i = -1;
	pids = make_pid_tab(cmds_get_n(shell));
	shell->cmd = shell->cmd_head;
	while (shell->cmd->next)
	{
		pipe(shell->pipefd);
		if (shell->cmd->fd_out == 1)
			shell->cmd->fd_out = shell->pipefd[1];
		pids[++i] = pipexec(shell, shell->pipefd[0], envp, pids);
		close_cmd_fds(shell->cmd);
		close(shell->pipefd[1]);
		shell->cmd = shell->cmd->next;
		if (shell->cmd->fd_in == 0)
			shell->cmd->fd_in = shell->pipefd[0];
		else
			close(shell->pipefd[0]);
	}
	pids[++i] = pipexec(shell, -1, envp, pids);
	close_cmd_fds(shell->cmd);
	return (ft_wait(pids, shell));
}
