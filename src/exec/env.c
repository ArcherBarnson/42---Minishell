/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgrulois <bgrulois@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/12 17:39:27 by bgrulois          #+#    #+#             */
/*   Updated: 2023/01/25 18:46:03 by mbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/minishell.h" 

char	*export_format(char *var)
{
	char	*formated_var;

	formated_var = ft_strjoin("export ", var);
	return (formated_var);
}

int	env_var_exists(char *str, char **envpc)
{
	int	i;

	i = 0;
	if (!str || !envpc)
		return (0);
	while (envpc[i] != NULL)
	{
		if (ft_exportcmp(str, envpc[i]))
			return (1);
		i++;
	}
	return (0);
}

int	env(t_shell *shell, char **envp, int mode)
{
	int		i;
	char	*export_var;

	i = 0;
	if ((mode == 0 && get_tab_size(shell->cmd->token) != 1) || !envp)
		return (0);
	while (envp[i] != NULL)
	{
		if (mode == 0 && is_env_var(envp[i]))
		{
			write(shell->cmd->fd_out, envp[i], ft_strlen(envp[i]));
			write(shell->cmd->fd_out, "\n", 1);
		}
		else if (mode == 1)
		{
			export_var = export_format(envp[i]);
			write(shell->cmd->fd_out, export_var, ft_strlen(export_var));
			write(shell->cmd->fd_out, "\n", 1);
			free(export_var);
		}
		i++;
	}
	return (0);
}

char	*check_plus_in_exp(char *exp, int mode)
{
	int		i;
	char	*save;

	if (mode != 2)
		save = ft_strdup(exp);
	else
	{
		i = 0;
		while (exp[i])
		i++;
		save = malloc(sizeof(char) * i);
		if (!save)
			return (NULL);
		i = -1;
		while (exp[++i] != '+')
			save[i] = exp[i];
		while (exp[++i])
			save[i - 1] = exp[i];
		save[i - 1] = '\0';
	}
	return (save);
}
