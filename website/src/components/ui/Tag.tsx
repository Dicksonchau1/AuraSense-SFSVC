interface TagProps {
  label: string;
  variant?: 'default' | 'accent' | 'status';
}

export function Tag({ label, variant = 'default' }: TagProps) {
  const variants = {
    default: 'border-border-default text-text-secondary',
    accent: 'border-accent-primary/30 text-accent-primary',
    status: 'border-accent-secondary/30 text-accent-secondary',
  };

  return (
    <span className={`inline-flex items-center px-3 py-1 text-xs font-mono tracking-wide border rounded-sm ${variants[variant]}`}>
      {label}
    </span>
  );
}
